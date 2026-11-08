srcs = files(
  'Prism/Algorithm/Hash.cpp',

  'Prism/Containers/Bitmap.cpp',
  'Prism/Containers/RingBuffer.cpp',

  'Prism/Debug/Assertions.cpp',
  'Prism/Debug/Log.cpp',
  'Prism/Debug/Logger.cpp',
  'Prism/Debug/LogSink.cpp',
  'Prism/Debug/Ubsan.cpp',
  'Prism/Debug/Stacktrace.cpp',

  'Prism/Memory/Memory.cpp',
  'Prism/String/Printf.cpp',

  'Prism/Utility/Path.cpp',
  'Prism/Utility/PathView.cpp',
  'Prism/Utility/Time.cpp',
)

# Unit testing framework (hosted only: uses fork(), clock_gettime(), ...).
test_framework_srcs = files(
  'Prism/Test/CrashTest.cpp',
  'Prism/Test/TestSuite.cpp',
)
test_main_srcs = files('Prism/Test/TestMain.cpp')

incs += include_directories('.')
