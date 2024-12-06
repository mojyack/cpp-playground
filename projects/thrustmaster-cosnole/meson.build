project('console', 'cpp', version : '1.0.0', default_options : ['warning_level=3', 'werror=false', 'cpp_std=c++20'])
add_project_arguments('-Wno-missing-field-initializers', language : 'cpp')

executable('console', files(
    'src/caps.cpp',
    'src/console.cpp',
    'src/input-decoder.cpp',
    'src/state-encoder.cpp',
    'src/udp.cpp',
  ),
)

executable('receptor', files(
    'src/receptor.cpp',
    'src/udp.cpp',
  ),
)
