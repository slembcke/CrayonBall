#AS::LIB_DIR = File.expand_path('../../Aerosol')

SCRIPT_LINES__ = {}

puts "Resources/init.rb: adding starters/ to the search path"
$: << '../starters'

puts "Resources/init.rb: (#{Dir.pwd})"
