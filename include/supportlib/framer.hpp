// framer.h                                                           -*-C++-*-
#ifndef SUPPORTLIB_FRAMER_H_INCLUDED
#define SUPPORTLIB_FRAMER_H_INCLUDED

#include <iostream>
#include <cstring>
#include <string>

struct Framer {
    explicit Framer(const char *title, bool verbose = false)
    : m_length_(std::strlen(title) + (verbose ? 9 : 0))
    {
        std::cout << "\n----------------- " << title;
        if (verbose) {
            std::cout << " (verbose)";
        }
        std::cout << " -----------------\n";
    }

    ~Framer()
    {
        std::cout << "------------------" << std::string(m_length_, '-')
                  << "------------------\n";
    }

private:
    size_t m_length_;
};

#endif

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
