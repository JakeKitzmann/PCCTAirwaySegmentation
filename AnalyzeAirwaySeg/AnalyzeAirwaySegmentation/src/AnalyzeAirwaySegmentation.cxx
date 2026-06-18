#include <AnalyzeAirwaySegmentationCLP.h>

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    PARSE_ARGS;

    // Locate evaluate.py next to this binary
    char exe_buf[4096];
    ssize_t n = readlink("/proc/self/exe", exe_buf, sizeof(exe_buf) - 1);
    if (n < 0) {
        std::cerr << "Failed to determine executable path\n";
        return 1;
    }
    exe_buf[n] = '\0';
    std::string exe_dir(exe_buf);
    exe_dir = exe_dir.substr(0, exe_dir.rfind('/'));
    std::string script = exe_dir + "/evaluate.py";

    std::vector<const char*> args = {
        PYTHON_EXECUTABLE,
        script.c_str(),
        "--segmentationGT",  segmentationGT.c_str(),
        "--segmentationPred", segmentationPred.c_str(),
        "--skeletonGT",      skeletonGT.c_str(),
        "--skeletonPred",    skeletonPred.c_str(),
        "--labeledGT",       labeledGT.c_str(),
        "--labeledPred",     labeledPred.c_str(),
        nullptr
    };

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "fork() failed\n";
        return 1;
    }
    if (pid == 0) {
        // Slicer sets PYTHONHOME/PYTHONPATH to its bundled Python; unset them
        // so the system python3 finds its own stdlib instead of Slicer's.
        unsetenv("PYTHONHOME");
        unsetenv("PYTHONPATH");
        execvp(PYTHON_EXECUTABLE, const_cast<char* const*>(args.data()));
        _exit(127);
    }
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
