#include <pstring_stage4.h>

#include <supportlib/framer.h>
#include <supportlib/assert.h>

#include <memory_resource_p1160>

void test(bool verbose)
{
    Framer framer{ "Stage4", verbose };

    std::pmr::test_resource tpmr{ "stage4", verbose };
    tpmr.set_no_abort(true);

    pstring astring{ "barfool", &tpmr };

    ASSERT_EQ(astring.str(), "barfool");
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
