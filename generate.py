import numpy as np

it = np.uint32
dt = np.dtype(np.uint32)


def generate(formatted_name, upper_bound, array_size):
    value_list = np.random.randint(0, high=upper_bound, size=array_size, dtype=it)
    value_list = np.sort(value_list, kind="quicksort")

    print("sampled values.")
    # print(value_list)

    # write bytes directly into the file
    # note: on Intel this is little endian

    with open(F"tests/{formatted_name}.case", "wb") as f:
        f.write(value_list.tobytes())

    # randomly select some quantiles in the array
    present_values_quantile = np.quantile(value_list, [ (1 / query_size) * i for i in range(query_size + 1) ], method='nearest')
    print("sampled quantiles.")
    # randomly select some present in the array
    present_values = np.sort(np.random.choice(value_list, size=query_size), kind="quicksort")
    print("sampled present values.")

    non_present_values = np.sort(np.random.randint(0, size=(query_size * 2), high=upper_bound, dtype=it))
    num_attempts = 0
    print(F"sampling non array.", end="\r")

    while np.isin(non_present_values, value_list).any():
        non_present_values = np.sort(np.random.randint(0, size=(query_size * 2), high=upper_bound, dtype=it))
        print(F"resampling non array. {num_attempts} attempt(s)", end="\r")
        num_attempts += 1
    print()
    np.sort(non_present_values, kind="quicksort")
    # print(F"after sampling: {non_present_values}")
    

    # write one present and one not-present
    with open(F"tests/{formatted_name}.quan", "wb") as f:
        f.write(present_values_quantile.tobytes())
    with open(F"tests/{formatted_name}.pres", "wb") as f:
        f.write(present_values.tobytes())
    with open(F"tests/{formatted_name}.non", "wb") as f:
        f.write(non_present_values.tobytes())

    # print(present_values_quantile)
    # print(present_values)
    

    return value_list, present_values_quantile, present_values, non_present_values

def verify(formatted_name, d_type, value_list, present_values_quantile, present_values, non_present_values):
    contents = None

    non_contents = None
    with open(F"tests/{formatted_name}.non", "rb") as f:
        non_contents  = f.read()
    non_values = np.frombuffer(non_contents, dtype=d_type)
    # print(F"non_value readback size: {non_values.size}")
    # print(F"non_value: {non_values}")

    with open(F"tests/{formatted_name}.case", "rb") as f:
        contents = f.read()
    print(formatted_name)
    read_value = np.frombuffer(contents, dtype=d_type)

    quan_contents = None
    with open(F"tests/{formatted_name}.quan", "rb") as f:
        quan_contents = f.read()
    quan_values = np.frombuffer(quan_contents, dtype=d_type)

    pres_contents = None
    with open(F"tests/{formatted_name}.pres", "rb") as f:
        pres_contents = f.read()
    pres_values = np.frombuffer(pres_contents, dtype=d_type)
    
    

    if np.equal(read_value, value_list).all():
        print("Content Equality check: Pass")
    else:
        print("Content Equality check: Failed")

    if np.equal(quan_values, present_values_quantile).all():
        print("Quan Equality check: Pass")
    else:
        print("Quan Equality check: Failed")
    
    if np.equal(pres_values, present_values).all():
        print("Present Equality check: Pass")
    else:
        print("Present Equality check: Failed")
    
    if np.equal(non_values, non_present_values).all():
        print("Non-pres Equality check: Pass")
    else:
        print("Non-pres Equality check: Failed")


    if np.isin(present_values_quantile, read_value).all() :
        print("Present quantiles values check: Pass")
    else:
        print("Present quantiles values check: Failed")
    
    if np.isin(present_values, read_value).all() :
        print("Present values check: Pass")
    else:
        print("Present values check: Failed")

    if not np.isin(non_present_values, read_value).any() :
        print("Non-Present values check: Pass")
    else:
        print("Non-Present values check: Failed")

    is_sorted = lambda a: np.all(a[:-1] <= a[1:])
    if is_sorted(non_present_values):
        print("Non-Present sorted check: Pass")
    else:
        print("Non-Present sorted: Failed")

    # print(read_value)

# for p in range(16):
#     pow_of_two = p
#     # max of uint_32 
#     upper_bound = (2 ** 32) - 1
#     # array_size = 15
#     array_size = (2 ** pow_of_two) * 1024 * 5
#     query_size = 10

#     name = "chunked-random"
#     formatted_name = F"n-{name}-s-{pow_of_two}-w-{dt.name}"
#     print(F"generating for power {p}")
#     value_list, present_values_quantile, present_value, non_present_value = generate(formatted_name, upper_bound, array_size)
#     verify(formatted_name, dt, value_list, present_values_quantile, present_value, non_present_value)
#     print("===================================================")

pow_of_two = 16
# max of uint_32 
upper_bound = (2 ** 32) - 1
# array_size = 15
array_size = (2 ** pow_of_two) * (2 ** 10) * 5
query_size = 10

name = "chunked-random"
formatted_name = F"n-{name}-s-{pow_of_two}-w-{dt.name}"
print(F"generating for power {pow_of_two}")
value_list, present_values_quantile, present_value, non_present_value = generate(formatted_name, upper_bound, array_size)
verify(formatted_name, dt, value_list, present_values_quantile, present_value, non_present_value)
