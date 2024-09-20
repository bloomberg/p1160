#include <supportlib/framer.hpp>
#include <supportlib/assert.hpp>

#include <pstring_stage9.hpp>

#include <pmr/test_resource.hpp>

#include <utility>

void test(bool verbose)
{
    Framer framer{ "Monitoring", verbose };

    beman::pmr::test_resource           dr{ "default" };
    beman::pmr::test_resource_monitor  drm{ dr };
    beman::pmr::default_resource_guard drg{ &dr };
    dr.set_verbose(verbose);

    beman::pmr::test_resource          tr{ "object" };
    beman::pmr::test_resource_monitor trm{ tr };
    tr.set_verbose(verbose);

    pstring astring{ "barfool", &tr };
    ASSERT(trm.is_total_up());
    ASSERT_EQ(trm.delta_blocks_in_use(), 1);
    trm.reset();

    pstring mstring{ std::move(astring) };

    ASSERT(drm.is_in_use_same()); // Did not allocate`bstring` with this
    ASSERT(drm.is_total_same());  // Did not allocate /anything/ with this
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
