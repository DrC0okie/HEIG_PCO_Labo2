import itertools
import hashlib

alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!$~*"

k = int(input("Enter the length of the password you want to generate: "))
N = int(input("Enter the desired thread count: "))
salt = input("Enter the salt: ")

# Generate the indexes of the permutations
permutations = len(alphabet) ** k
indexes = []
for _ in range(N):
    permutations = int(permutations / 2)
    indexes.append(permutations)

# Generate all permutations of the alphabet with length k
permutations = list(itertools.product(alphabet, repeat=k))

for i in indexes:
    password = ''.join(permutations[i - 1])
    salted_password = salt + password
    hashed_password = hashlib.md5(salted_password.encode()).hexdigest()
    print(f"Password at position {i}: {password} (salted: {salted_password}, hashed: {hashed_password})")

