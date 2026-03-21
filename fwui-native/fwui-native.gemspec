# frozen_string_literal: true

Gem::Specification.new do |s|
  s.name        = 'fwui-native'
  s.version     = '1.0.0'
  s.summary     = 'Native C extension for FWUI HTML rendering'
  s.description = 'C extension for FWUI: direct ivar access (ROBJECT_IVPTR), ' \
                  'render caching, baked templates, and Inja template engine. ' \
                  '~3x cold render, instant cached, 250x+ with registry cache.'

  s.authors  = ['Vaniello']
  s.email    = ['ripaivan11@gmail.com']
  s.homepage = 'https://github.com/Vaniell0/fwui'
  s.license  = 'BUSL-1.1'

  s.required_ruby_version = '>= 3.0.0'

  s.add_runtime_dependency 'fwui', '~> 1.0'

  s.files         = Dir['ext/**/*.{c,cpp,h,hpp,rb}', 'ext/**/vendor/**/*.hpp', 'lib/**/*.rb']
  s.extensions    = ['ext/fwui_native/extconf.rb']
  s.require_paths = ['lib']
end
