module AS::ResourceCache
	LoadedResource = Struct.new(:obj, :loader)
	LoadedResources = {}
	Loaders = []
	
	def self.load(*key, &rule_exception)
		#unpack single items from the varargs
		# [5] => 5, [5,6] => [5,6]
		key = *key
		
		return LoadedResources[key].obj if LoadedResources.key?(key)
		
		# get the loader block to load the resource
		loader = (rule_exception or Loaders.find{|loader| loader.match?(key)}.loader)
		# load, and cache the object
		obj = loader.call(key)
		LoadedResources[key] = LoadedResource.new(obj, loader)
		
		return obj
	end
	
	def self.reload
		LoadedResources.each do|key, resource|
			resource.obj = resource.rule.load(key)
		end
	end
end

class AS::LazyLoader
	AS::ResourceCache::Loaders << self
	
	attr_reader :block
	
	def initialize(&block)
		@block = block
	end
	
	def self.match?(key)
		self === key
	end
	
	def self.loader
		lambda{|key| puts "loading #{key.inspect}"; key.block.call}
	end
	
	def load
		AS::ResourceCache.load(self)
	end
end
