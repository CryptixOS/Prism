#*
#* Created by v1tr10l7 on 01.03.2025.
#* Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-3
#*/

cpp_args = [
  '-Wno-unused-parameter',
  '-Wno-self-assign-overloaded',
  '-lgcc_s',
  '-fmodules-ts',
  '-DPRISM_DISABLE_FMT=0'
]
link_args = [
  '-lstdc++exp',
]

extraincs = get_option('extra_incs')
if extraincs != ''
  cpp_args += '-I' + extraincs
endif

subdir('Algorithm')
subdir('Core')
subdir('Containers')
subdir('Debug')
subdir('Memory')
subdir('String')
subdir('Utility')

