import argparse
import json
import matplotlib.pyplot as plt
import numpy as np

def get_json(file_path):
    with open(file_path, "r") as f:
        return json.load(f)

def get_algo_name(cmd):
    return cmd.split(" ")[0].removeprefix("./build/src/algos/")

def get_summary_point(collection, iter_idx, query_type, file_power):
    file_name = F"n-chunked-random-s-{file_power:02}-w-uint32-result.json"
    file_path = F"results/{query_type}{iter_idx}/{file_name}"

    bench_json = get_json(file_path)

    for result in bench_json["results"]:
        if get_algo_name(result["command"]) not in collection:
            collection[get_algo_name(result["command"])] = {}

        collection[get_algo_name(result["command"])][file_power] \
            = { "mean": float(result["mean"]), 
                "stddev": float(result["stddev"]),
                "max": float(result["max"]) } 

def get_timings_point(iter_idx, query_type, file_power):
    file_name = F"n-chunked-random-s-{file_power:02}-w-uint32-result.json"
    file_path = F"results/{query_type}{iter_idx}/{file_name}"

    bench_json = get_json(file_path)

    collection = {}

    for result in bench_json["results"]:
        if get_algo_name(result["command"]) not in collection:
            collection[get_algo_name(result["command"])] = {}

        collection[get_algo_name(result["command"])] = result["times"]

    return collection

def get_summaries_size_axis(iter_idx, query_type):
    summaries = {}

    for file_power in range(18):
        get_summary_point(summaries, iter_idx, query_type, file_power)
    return summaries

# def get_timings_size_axis(iter_idx, query_type):
#     timings = {}

#     for file_power in range(18):
#         get_timings_point(timings, iter_idx, query_type, file_power)
#     return timings

def into_mean_array(points_dict):
    # assumes we start from 0, which we do
    arr = [0] * len(points_dict.keys()) 

    for key in points_dict.keys():
        arr[int(key)] = points_dict[key]["mean"]
    
    return arr


###############################################
"""
solution: get average timings scale with file size
query type: fixed
position of query: fixed
file_size: varied
"""
# Example
rough_position_marker = 10
query_type = "n"
d_points = get_summaries_size_axis(rough_position_marker, query_type = "n")
scaling_times_np_binary_basic = np.asarray(into_mean_array(d_points["binary_search_basic"]))
scaling_times_np_linear_basic = np.asarray(into_mean_array(d_points["linear_search_basic"]))
scaling_times_np_search_vector = np.asarray(into_mean_array(d_points["linear_search_vector"]))
scaling_times_np_linear_basic_early_term = np.asarray(into_mean_array(d_points["linear_search_basic_early_term"]))
scaling_times_np_search_vector_early_term = np.asarray(into_mean_array(d_points["linear_search_vector_early_term"]))

X = np.arange(0, 18, 1)
plt.plot(X, scaling_times_np_binary_basic, color='r', label='binary_search_basic')
plt.plot(X, scaling_times_np_linear_basic, color='g', label='linear_search_basic')
plt.plot(X, scaling_times_np_search_vector, color='b', label='linear_search_vector')
plt.plot(X, scaling_times_np_linear_basic_early_term, color='c', label='linear_search_basic_early_term')
plt.plot(X, scaling_times_np_search_vector_early_term, color='y', label='linear_search_vector_early_term')

"""
solution: get timings for whisker plot
query type: fixed
position of query: fixed
file_size: fixed
"""
# Example 
rough_position_marker = 19
query_type = "n"
file_size = 15
timings = get_timings_point(rough_position_marker, query_type, file_size)

timing_np_binary_basic = np.asarray(timings["binary_search_basic"])
timing_np_linear_basic = np.asarray(timings["linear_search_basic"])
timing_np_search_vector = np.asarray(timings["linear_search_vector"])
timing_np_linear_basic_early_term = np.asarray(timings["linear_search_basic_early_term"])
timing_np_search_vector_early_term = np.asarray(timings["linear_search_vector_early_term"])

timings_fig, axs = plt.subplots(1, 5)
axs[0].boxplot(timing_np_binary_basic)
axs[0].set_title('binary_search_basic')

axs[1].boxplot(timing_np_linear_basic)
axs[1].set_title('linear_search_basic')

axs[2].boxplot(timing_np_search_vector)
axs[2].set_title('linear_search_vector')

axs[3].boxplot(timing_np_linear_basic_early_term)
axs[3].set_title('linear_search_basic_early_term')

axs[4].boxplot(timing_np_search_vector_early_term)
axs[4].set_title('linear_search_vector_early_term')

timings_fig.subplots_adjust(left=0.08, right=2.98, bottom=0.05, top=1.5,
                    hspace=0.4, wspace=0.3)




###############################################
