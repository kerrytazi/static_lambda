#docker run -it --rm -v ${PWD}:/project clang-lldb-image /usr/bin/bash

#docker run -it --rm -v ${PWD}:/project clang-lldb-image /usr/bin/bash -c "mkdir -p build_linux/Debug && cd build_linux/Debug && cmake -Dtest=1 -DCMAKE_BUILD_TYPE=Debug ../.. && cmake --build ."
#docker run -it --rm -v ${PWD}:/project clang-lldb-image /usr/bin/bash -c "mkdir -p build_linux && cd build_linux && cmake -Dtest=1 .. && cmake --build . --config RelWithDebInfo"

#docker run -it --rm -v ${PWD}:/project clang-lldb-image /usr/bin/bash -c build_linux/Debug/static_lambda_test

#docker run -it --rm --privileged --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -v ${PWD}:/project clang-lldb-image /usr/bin/bash -c "lldb build_linux/static_lambda_test"
#docker run -it --rm --privileged --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -p 1234:1234 -v ${PWD}:/project clang-lldb-image lldb-server platform --server --listen *:1234
#docker run -it --rm --privileged --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -p 1234:1234 -v ${PWD}:/project clang-lldb-image lldb-server gdbserver *:1234 build_linux/Debug/static_lambda_test
#docker run -it --rm --privileged --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -p 1234:1234 -v ${PWD}:/project clang-lldb-image lldb-server gdbserver *:1234

docker build -t clang-lldb-image ./test
