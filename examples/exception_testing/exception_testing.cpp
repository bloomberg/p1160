#include <supportlib/framer.hpp>
#include <supportlib/assert.hpp>

#include <pmr/test_resource.hpp>

#include <deque>
#include <string>

void test(bool verbose)
{
    Framer framer{ "Exception Testing", verbose };

    beman::pmr::test_resource tpmr{ "tester", verbose };
    const char* longstr = "A very very long string that allocates memory";

    for (auto context : tpmr.exception_test_range()) {
        std::pmr::deque<std::pmr::string> deq{ { "hello", "world"}, &tpmr};
        std::pmr::deque<std::pmr::string> orig{ deq };

        context.run_test([&]() {
                deq.emplace_back(longstr);
                orig = deq;
                deq.emplace_back(longstr);
                ASSERT_EQ(deq.size(), 4);
            },
            [&](const beman::pmr::test_resource_exception&) {
                ASSERT_EQ(deq.size(), orig.size());
            });
    }
}

int main()
{
    test(false);
    test(true);
}

// ----------------------------------------------------------------------------
// Copyright 2019 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License") with the LLVM
// exception (the "Exception"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License and the Exception at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//     https://spdx.org/licenses/LLVM-exception.html
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
