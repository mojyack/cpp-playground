project('lidar', ['c', 'cpp'], version : '1.0.0', default_options : ['warning_level=3', 'werror=false', 'cpp_std=c++20'])
add_project_arguments('-Wno-missing-field-initializers', language : 'cpp')

if get_option('gui')
  subdir('src/gawl')

  executable('lidar', files(
      'src/main.cpp',
      'src/payloader.cpp',
      'src/udp.cpp',
      'src/lidar.cpp',
    ) + gawl_core_files,
    dependencies : [
      dependency('YDLIDAR_SDK'),
    ] + gawl_core_deps,
  )
endif

executable('transmitter', files(
    'src/payloader.cpp',
    'src/transmitter.cpp',
    'src/udp.cpp',
    'src/lidar.cpp',
  ),
  dependencies : [
    dependency('YDLIDAR_SDK'),
  ],
)
