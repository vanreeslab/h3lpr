/*
 * Copyright (c) Massachusetts Institute of Technology
 *
 * See LICENSE in top-level directory
 */
#include "profiler.hpp"

#include <mpi.h>
#include <stack>

using std::map;
using std::string;

namespace H3LPR {

static constexpr int    upper_rank = 1000; // approximates the infinity of procs
static map<int, double> t_nu       = {{0, 0.0},
                                   {1, 6.314},
                                   {2, 2.920},
                                   {3, 2.353},
                                   {4, 2.132},
                                   {5, 2.015},
                                   {7, 1.895},
                                   {10, 1.812},
                                   {15, 1.753},
                                   {20, 1.725},
                                   {30, 1.697},
                                   {50, 1.676},
                                   {100, 1.660},
                                   {upper_rank, 1.645}};
/**
 * @brief return the t_nu for 90% confidence interval width based on the interpolation of the above table
 * 
 * @param nu the number of proc-1
 * @return double the confidence interval param
 */
double t_nu_interp(const int nu) {
    m_assert_h3lpr(nu >= 0, "the nu param = %d must be positive", nu);
    //--------------------------------------------------------------------------
    if (nu == 0) {
        // easy, it's 0
        return 0.0;
    } else if (nu <= 5) {
        // we have an exact entry
        const auto it = t_nu.find(nu);
        return it->second;
    } else if (nu >= upper_rank) {
        // we are too big, it's like a normal distribution
        return 1.645;
    } else {
        // find the right point
        auto         it_up  = t_nu.lower_bound(nu);  // first element >= nu
        const int nu_up  = it_up->first;
        const double t_up   = it_up->second;
        auto         it_low = std::prev(it_up,1);  // take the previous one
        const int nu_low = it_low->first;
        const double t_low  = it_low->second;
        // m_log("nu_up = %d, nu_low = %d, t_up=%f, t_low=%f",nu_up,nu_low,t_up, t_low);
        return t_low + (t_up-t_low)/(nu_up-nu_low)*(nu-nu_low);
    }
    //--------------------------------------------------------------------------
}

/**
 * @brief defines a simple block with a given name
 */
TimerBlock::TimerBlock(string name) {
    //--------------------------------------------------------------------------
    name_  = name;
    t0_    = -1.0;
    t1_    = -1.0;
    count_ = 0;
    //--------------------------------------------------------------------------
}

/**
 * @brief deletes the current TimeBlock and delete its children 
 * 
 */
TimerBlock::~TimerBlock() {
    //--------------------------------------------------------------------------
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        delete it->second;
    }
    //--------------------------------------------------------------------------
}

/**
 * @brief start the timer using MPI_Wtime
 * 
 */
void TimerBlock::Start() {
    m_assert_h3lpr(t0_ < -0.5, "the block %s has already been started", name_.c_str());
    count_ += 1;
    t0_ = MPI_Wtime();
}

/**
 * @brief start the timer without incrementing the call count
 * 
 */
void TimerBlock::Resume() {
    m_assert_h3lpr(t0_ < -0.5, "the block %s has already been started", name_.c_str());
    t0_ = MPI_Wtime();
}

// /**
//  * @brief start the timer using the time provided as argument
//  * 
//  */
// void TimerBlock::Start(const double stop_time) {
//     m_assert_h3lpr(t0_ < -0.5, "the block %s has already been started", name_.c_str());
//     count_ += 1;
//     t0_ = stop_time;
// }

// /**
//  * @brief stop the timer using MPI_Wtime
//  * 
//  */
// void TimerBlock::Stop() {
//     // get the time
//     t1_ = MPI_Wtime();
//     // 
//     m_assert_h3lpr(t0_ > -0.5, "the block %s is stopped without being started", name_.c_str());
//     // store it
//     double dt = t1_ - t0_;
//     time_acc_ = time_acc_ + dt;
//     // reset to negative for the checks
//     t0_ = -1.0;
//     t1_ = -1.0;
// }

/**
 * @brief stop the timer using the time provided as argument
 * 
 */
void TimerBlock::Stop(const double time) {
    // get the time
    t1_ = time;
    // 
    m_assert_h3lpr(t0_ > -0.5, "the block %s is stopped without being started", name_.c_str());
    // store it
    double dt = t1_ - t0_;
    time_acc_ = time_acc_ + dt;
    // reset to negative for the checks
    t0_ = -1.0;
    t1_ = -1.0;
}

/**
 * @brief adds a child to my list of children
 * 
 */
TimerBlock* TimerBlock::AddChild(string child_name) noexcept {
    // find cleanly the child
    auto it = children_.find(child_name);

    if (it == children_.end()) {
        TimerBlock* child     = new TimerBlock(child_name);
        children_[child_name] = child;
        child->SetParent(this);
        return child;
    } else {
        return it->second;
    }
}

/**
 * @brief returns the time of the requested children
 * 
 * @param child_name 
 * @return double 
 */
double TimerBlock::GetChildrenTime(string child_name) noexcept{
    auto it = children_.find(child_name);
    m_assert_h3lpr(it != children_.end(),"you requested the time of %s which is not a child",child_name.c_str());
    return it->second->time_acc();
}

/**
 * @brief store the parent pointer
 * 
 * @param parent 
 */
void TimerBlock::SetParent(TimerBlock* parent) {
    parent_ = parent;
    // is_root_ = false;
}

/**
 * @brief display the time accumulated
 * 
 * If the timer is a ghost timer (never called but still created), we return the sum on the children
 * 
 * @return double 
 */
double TimerBlock::time_acc() const {
    if (count_ > 0) {
        return time_acc_;
    } else {
        double sum = 0.0;
        for (auto it = children_.cbegin(); it != children_.cend(); ++it) {
            const TimerBlock* child = it->second;
            sum += child->time_acc();
        }
        return sum;
    }
}

/**
 * @brief display the time for the TimerBlock
 * 
 * @param file pointer to the file to write the results
 * @param level the indentation level
 * @param total_time the total time used to compute percentages
 */
void TimerBlock::Disp(FILE* file, const int level, const double total_time, const int icol) const {
    // check if any proc has called the agent
    int total_count = 0;
    MPI_Allreduce(&count_, &total_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    // get the size and useful stuffs
    int comm_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

// setup the displayed name
#if (M_COLOR_PROF)
    string shifter = "\033[0;35m";
#else
    string shifter;
#endif
    for (int l = 1; l < level - 1; l++) {
        shifter = shifter + "|   ";
    }
    if (level > 1) {
        shifter = shifter + "|-> ";
    }
// string myname = shifter + "\033[0m" + "\033[1m" + name_ + "\033[0m";
#if (M_COLOR_PROF)
    string myname = shifter + "\033[0m" + name_;
#else
    string myname = shifter + name_;
#endif

    //................................................
    // compute my numbers
    if (total_count > 0) {
        // compute the counters (mean, max, min)
        double local_count = count_;
        double max_count = 0.0, min_count = 0.0, mean_count = 0.0;
        mean_count = total_count / comm_size;
        MPI_Allreduce(&local_count, &min_count, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
        MPI_Allreduce(&local_count, &max_count, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

        // compute times passed inside + children
        double local_time = time_acc_;
        double sum_time = 0.0, min_time = 0.0, max_time = 0.0;
        MPI_Allreduce(&local_time, &sum_time, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        MPI_Allreduce(&local_time, &min_time, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
        MPI_Allreduce(&local_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

        double mean_time           = sum_time / comm_size;
        double mean_time_per_count = sum_time / total_count;
        double glob_percent        = mean_time / total_time * 100.0;

        // confidence interval 90% using the t distribution
        double sum_timesq;
        double local_timesq = (local_time - mean_time) * (local_time - mean_time);
        MPI_Allreduce(&local_timesq, &sum_timesq, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        double std_time   = (comm_size > 1) ? (sqrt(sum_timesq / (comm_size - 1))) : 0.0;
        double ci_90_time = (comm_size > 1) ? (std_time / sqrt(comm_size) * t_nu_interp(comm_size - 1)) : 0.0;

        // printf the important information
        if (rank == 0) {
#if (M_COLOR_PROF)
            // printf("%-25.25s|  %9.4f\t%9.4f\t%9.6f\t%9.6f\t%9.6f\t%9.6f\t%9.6f\t%09.1f\t%9.2f\n", myname.c_str(), glob_percent, loc_percent, mean_time, self_time, mean_time_per_count, min_time_per_count, max_time_per_count, mean_count, mean_bandwidth);
            if (icol == 0) {  // go red
                printf("%-60.60s %s\033[0;31m%09.6f\033[0m %% -> \033[0;31m%07.4f\033[0m [s] +- %07.4f [s] \t\t\t(%.4f [s/call], %.0f calls)\n", myname.c_str(), shifter.c_str(), glob_percent, mean_time, ci_90_time, mean_time_per_count, max_count);
            }
            if (icol == 1) {  // go orange
                printf("%-60.60s %s\033[0;33m%09.6f\033[0m %% -> \033[0m%07.4f\033[0m [s] +- %07.4f [s] \t\t\t(%.4f [s/call], %.0f calls)\n", myname.c_str(), shifter.c_str(), glob_percent, mean_time, ci_90_time, mean_time_per_count, max_count);
            }
            if (icol == 2) {  // go normal
                printf("%-60.60s %s\033[0m%09.6f\033[0m %% -> \033[0m%07.4f\033[0m [s] +- %07.4f [s] \t\t\t(%.4f [s/call], %.0f calls)\n", myname.c_str(), shifter.c_str(), glob_percent, mean_time, ci_90_time, mean_time_per_count, max_count);
            }
#else
            printf("%-60.60s %s%09.6f %% -> %07.4f [s] +- %07.4f [s] \t\t\t(%.4f [s/call], %.0f calls)\n", myname.c_str(), shifter.c_str(), glob_percent, mean_time, ci_90_time, mean_time_per_count, max_count);
#endif
            // printf in the file
            if (file != nullptr) {
                fprintf(file, "%s;%d;%.8f;%.8f;%.8f;%.0f;%.8f;%.8f;%.8f;%.0f;%.0f\n", name_.c_str(), level, mean_time, glob_percent, mean_time_per_count, mean_count, min_time, max_time, std_time, min_count, max_count);
            }
        }
    } else if (name_ != "root") {
        // we have a total count = 0, nothing to do for the counter
        if ((rank == 0) && (file != nullptr)) {
            fprintf(file, "%s;%d;%.8f;%.8f;%.8f;%.0f;%.8f;%.8f;%.8f;%.0f;%.0f\n", name_.c_str(), level, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        }
    }

    //................................................
    // check that everything is ok for the MPI
#if (M_DEBUG)
    int nchildren     = children_.size();
    int nchildren_max = 0;
    int nchildren_min = 0;
    MPI_Allreduce(&nchildren, &nchildren_max, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    MPI_Allreduce(&nchildren, &nchildren_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
    m_assert_h3lpr((nchildren_max == nchildren) && (nchildren == nchildren_min), "TimerBlock %s: nchildren do not match: local = %d, max = %d, min = %d", name_.c_str(), nchildren, nchildren_max, nchildren_min);
#endif

    //................................................
    // recursive call to the childrens
    // get the colors
    string max_name, min_name;
    double max_time = std::numeric_limits<double>::min();
    double min_time = std::numeric_limits<double>::max();
    for (auto it = children_.cbegin(); it != children_.cend(); ++it) {
        TimerBlock* child = it->second;
        double      ctime = child->time_acc();
        if (ctime > max_time) {
            max_time = ctime;
            max_name = child->name();
        }
        if (ctime < min_time) {
            min_time = ctime;
            min_name = child->name();
        }
    }
    for (auto it = children_.cbegin(); it != children_.cend(); ++it) {
        TimerBlock* child = it->second;
        if (child->name() == max_name && icol == 0) {
            // go red
            child->Disp(file, level + 1, total_time, 0);
        } else if (child->name() == max_name && icol > 0) {
            // go orange
            child->Disp(file, level + 1, total_time, 1);
        } else {
            child->Disp(file, level + 1, total_time, 2);
        }
    }
}

//===============================================================================================================================
/**
 * @brief Construct a new Prof with a default name
 */
Profiler::Profiler() : name_("default") {
    // create the current Timer Block "root"
    root_ = new TimerBlock("root");
    current_ = root_;
}

/**
 * @brief Construct a new Prof with a given name
 */
Profiler::Profiler(const string myname) : name_(myname) {
    // create the current Timer Block "root"
    root_ = new TimerBlock("root");
    current_ = root_;
}

/**
 * @brief Destroy the Prof
 */
Profiler::~Profiler() {
    if (current_ != root_) {
        std::string remaining_blocks = "";
        while(current_->parent() != nullptr) {
            remaining_blocks += current_->name() + ", ";
            current_ = current_->parent();
        }
        m_log_h3lpr("WARNING: destroying profiler, but not all timers were stopped (remaining: %s)", remaining_blocks.c_str());
    }
    delete root_;
}

/**
 * @brief initialize the timer and move to it + return the TimerBlock adress
 */
void Profiler::Init(string name)  noexcept{
    current_ = current_->AddChild(name);
}

/**
 * @brief start the timer of the TimerBlock
 */
void Profiler::Start(string name) noexcept {
    current_->Start();
}

/**
 * @brief stop the timer of the TimerBlock using the given walltime
 */
void Profiler::Stop(string name, const double wtime) noexcept {
    m_assert_h3lpr(name == current_->name(), "we are trying to stop %s which is not the most recent timer started = %s", name.c_str(), current_->name().c_str());
    current_->Stop(wtime);
}

/**
 * @brief go back to the parent of the present timer block
 */
void Profiler::Leave(string name)noexcept  {
    current_ = current_->parent();
}

/**
 * @brief returns the elapsed time for one of the children only
 *
 * @param name
 */
double Profiler::GetTime(string name) noexcept {
    return current_->GetChildrenTime(name);
}

/**
 * @brief display the whole profiler
 */
void Profiler::Disp() {
    // record current time
    const double wtime = MPI_Wtime();
    const bool root_call = (current_ == root_);

    int comm_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    FILE*  file;
    string folder = "./prof";

    MPI_Barrier(MPI_COMM_WORLD);
    //-------------------------------------------------------------------------
    /** - do the IO of the timing */
    //-------------------------------------------------------------------------
    string filename = "./prof/" + name_ + "_time.csv";
    if (rank == 0) {
        file = fopen(filename.c_str(), "w+");
    }

    // Save current state, stop all running blocks, and retrieve the root
    std::stack<TimerBlock*> call_stack;
    std::string stopped_string = "";
    while (current_->parent() != nullptr) {
        current_->Stop(wtime);
        call_stack.push(current_);
        stopped_string += current_->name() + ", ";
        current_ = current_->parent();
    }
    m_assert_h3lpr(current_ == root_, "Current block should be the root block.");
    if (!root_call) {
        m_log_h3lpr("WARNING: displaying profiler, but not all timers were stopped (remaining: %s)", stopped_string.c_str());
    }

    // get the global timing
    double total_time = current_->time_acc();

    // display the header
    if (rank == 0) {
        printf("===================================================================================================================================================\n");
#if (M_COLOR_PROF)
        printf("        PROFILER %s --> total time = \033[0;33m%.4f\033[m [s] \n\n", name_.c_str(), total_time);
#else
        printf("        PROFILER %s --> total time = %.4f [s] \n\n", name_.c_str(), total_time);
#endif
    }

    // display root with the total time, root is the only block which is common to everybody
    current_->Disp(file, 0, total_time, 0);

    // display footer
    if (rank == 0) {
        printf("===================================================================================================================================================\n");
        printf("WARNING:\n");
        printf("  - times are mean-time with their associated 90%% CI\n");
        printf("  - the percentage might not be consistent are they only reflect rank-0 timing\n");
#if (M_COLOR_PROF)
        printf("legend:\n");
        printf("  - \033[0;31mthis indicates the most expensive step of the most expensive operation\033[0m\n");
        printf("  - \033[0;33mthis indicates the most expensive step of the parent operation\033[0m\n");
#endif
        printf("===================================================================================================================================================\n");

        if (file != nullptr) {
            fclose(file);
        } else {
            printf("unable to open file for profiling <%s>!\n", filename.c_str());
        }
    }

    // Restart all the stopped blocks to restore the old state
    while (!call_stack.empty()) {
        current_ = call_stack.top();
        current_->Resume();
        call_stack.pop();
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
}

}; // namespace H3LPR
