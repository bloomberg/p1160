#include <pstring_stage7.hpp>

#include <supportlib/framer.hpp>
#include <supportlib/assert.hpp>

#include <test_resource.hpp>

void test(bool verbose)
{
    Framer framer{ "Stage7", verbose };

    std::pmr::test_resource tpmr{ "stage7", verbose };
    tpmr.set_no_abort(true);

    pstring astring{ "barfool", &tpmr };
    pstring string2{ "string", &tpmr };

    string2 = astring;

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
