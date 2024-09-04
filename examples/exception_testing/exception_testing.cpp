#include <supportlib/framer.hpp>
#include <supportlib/assert.hpp>

#include <test_resource.hpp>

#include <deque>
#include <string>

void test(bool verbose)
{
    Framer framer{ "Exception Testing", verbose };

    std::pmr::test_resource tpmr{ "tester", verbose };
    const char *longstr = "A very very long string that allocates memory";

    std::pmr::exception_test_loop(tpmr,
                                  [longstr](std::pmr::memory_resource& pmrp) {
        std::pmr::deque<std::pmr::string> deq{ &pmrp };
        deq.emplace_back(longstr);
        deq.emplace_back(longstr);

        ASSERT_EQ(deq.size(), 2);
    });
}

int main()
{
    test(false);

    test(true);
}

// ----------------------------------------------------------------------------
// Copyright 2019 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
