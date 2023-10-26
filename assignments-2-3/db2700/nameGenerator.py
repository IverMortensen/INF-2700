from faker import Faker

fake = Faker()

number = int(input("Number of names to generate: "))
print(f"Generating {number} names.")

# Generate 10,000 random first names
first_names = [fake.first_name() for _ in range(number)]
# Generate 10,000 random last names
last_names = [fake.last_name() for _ in range(number)]

with open('./src/names.h', mode='w') as file:
    file.write(f"int num_names = {number};\n")
    
	# Write first names to file
    file.write(f"char first_names[{number}][15] = " + "{")
    for name in first_names:
        file.write(f'"{name}", ')
    file.write("};\n")
    
	# Write last names to file
    file.write(f"char last_names[{number}][15] = " + "{")
    for name in last_names:
        file.write(f'"{name}", ')
    file.write("};")
    