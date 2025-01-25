project('Skadi', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

sources = ['Launch.cpp',
            'Source/App.cpp',
            'Source/Graphics/Rend.cpp',
            'Source/Graphics/VulkanInstance.cpp',
            'Source/Graphics/DisplayInstance.cpp',
            'Source/Graphics/ResourceManager.cpp',
            'Source/Resources/Loader.cpp',
            'Source/Input/Input.cpp',
            'Source/Core/ECS/EntityManager.cpp',
            'Source/Core/ECS/Scene.cpp',
            'Test/Tests.cpp']

#dep_spirv = meson.get_compiler('cpp').find_library('SPIRV')
deps = [dependency('vulkan'), dependency('glfw3'), dependency('assimp'), dependency('unordered_dense')]

exe = executable('Skadi', sources, dependencies : deps,
  install : true)

test('basic', exe)
