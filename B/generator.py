
import random

def print_tree_edges(prufer, m):
	vertices = m + 2
	vertex_set = [0] * vertices

	# Initialize the array of vertices
	for i in range(vertices):
		vertex_set[i] = 0

	# Number of occurrences of vertex in code
	for i in range(vertices - 2):
		vertex_set[prufer[i] - 1] += 1

	#print("\nThe edge set E(G) is:")

	j = 0

	# Find the smallest label not present in prufer[].
	for i in range(vertices - 2):
		for j in range(vertices):
			# If j+1 is not present in prufer set
			if vertex_set[j] == 0:
				# Remove from Prufer set and print pair.
				vertex_set[j] = -1
				print("{} {}".format(j + 1, prufer[i]))

				vertex_set[prufer[i] - 1] -= 1

				break

	j = 0

	# For the last element
	for i in range(vertices):
		if vertex_set[i] == 0 and j == 0:
			print("{} ".format(i + 1), end="")
			j += 1
		elif vertex_set[i] == 0 and j == 1:
			print("{}".format(i + 1))

def generate_random_tree(n):
	length = n - 2
	arr = [0] * length

	# Generate random array
	for i in range(length):
		arr[i] = random.randint(1, length + 1)

	print_tree_edges(arr, length)

# Driver code
import sys
n = int(sys.argv[1])
print(n)
generate_random_tree(n)

