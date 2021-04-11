#!/bin/sh

echo "run_tests.sh"
echo "Workspace: '$WORKSPACE'"

export CFLAGS="-g -O0 -Wall -W -fprofile-arcs -ftest-coverage"
export CXXFLAGS="-g -O0 -Wall -W -fprofile-arcs -ftest-coverage"
export LDFLAGS="-fprofile-arcs -ftest-coverage"

PATH_SOURCE="$WORKSPACE"
PATH_BUILD="$WORKSPACE/out"
PATH_GCOV="$PATH_BUILD/Testing/CoverageInfo"
PATH_COVERAGE_XML="$PATH_BUILD/coverage.xml"

pushd $WORKSPACE

rm -rf $PATH_BUILD
mkdir -p $PATH_BUILD

pushd $PATH_BUILD
cmake -DCMAKE_BUILD_TYPE=Debug "$WORKSPACE"
popd

ctest -S test.ctest
RESULT_CODE=$?

pushd "$PATH_GCOV"
gcovr -r "$PATH_SOURCE" -x "$PATH_COVERAGE_XML"
popd

popd

exit $RESULT_CODE
