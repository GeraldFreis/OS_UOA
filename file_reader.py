import pandas as pd

# reading in the file values:
df = pd.DataFrame(columns=["File", "Frames", "Replacement", "Page_Fault_rate"])
counter = 1
experiment_info = str
out = open("output_file.txt", 'r')
for line in out.readlines():
    if(counter % 2 == 0): # if even line then we have the fault rate
        out = line.split(" ")
        fault_rate = float(out[-1].strip('\n'))
        print(fault_rate)
        file = experiment_info[1]
        frames = experiment_info[3]
        replacement_type = experiment_info[5]
        new_df = pd.DataFrame.from_dict({"File": [file], "Frames": [frames], "Replacement": [replacement_type], "Page_Fault_rate": [fault_rate]})
        df = pd.concat([new_df, df])
        counter += 1
        experiment_info = str
    else:
        counter += 1
        experiment_info = line.split(" ")
df.to_csv("Experiments.csv")