#include <pstring_stage4.hpp>

#include <supportlib/framer.hpp>
#include <supportlib/assert.hpp>

#include <pmr/test_resource.hpp>

void test(bool verbose)
{
    Framer framer{ "Stage4a", verbose };

    beman::pmr::test_resource tpmr{ "stage4a", verbose };
    tpmr.set_no_abort(true);

    pstring astring{ "barfool", &tpmr };
    pstring string2{ astring };

    ASSERT_EQ(astring.str(), "barfool");
    ASSERT_EQ(string2.str(), "barfool");
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
