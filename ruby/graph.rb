class Graph
	def initialize(nodes, edges)
		@nodes = nodes
	
		@edges = Hash.new{|hsh, key| hsh[key] = []}
		edges.each do|a, b|
			@edges[a] << b
			@edges[b] << a
		end
	end
	
	def flood(node, marked_nodes = [])
		marked_nodes << node
		
		@edges[node].each do|other_node|
			flood(other_node, marked_nodes) unless marked_nodes.include?(other_node)
		end
		
		return marked_nodes
	end
	
	def islands
		list = []
		nodes = @nodes
		
		until nodes.empty?
			island = flood(nodes.first)
			nodes -= island
			list << island
		end
		
		return list
	end
end
