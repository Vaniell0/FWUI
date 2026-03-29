# frozen_string_literal: true

require 'mkmf'

# C flags for fwui_native.c
$CFLAGS << ' -O3 -Wall -Wextra -Wno-unused-parameter'

# C++ flags for fwui_inja.cpp (Inja requires C++17)
$CXXFLAGS << ' -O3 -std=c++17 -Wall -Wextra -Wno-unused-parameter'

# Vendored headers (inja.hpp, nlohmann/json.hpp)
vendor_dir = File.join(__dir__, 'vendor')
$INCFLAGS << " -I#{vendor_dir}"

# pthread for thread-safe baked template registry
have_library('pthread')

# Build both .c and .cpp sources
$srcs = Dir.glob(File.join(__dir__, '*.{c,cpp}'))

create_makefile('fwui_native/fwui_native')
