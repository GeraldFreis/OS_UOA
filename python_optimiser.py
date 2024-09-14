import os
current_dir = os.getcwd()
# os.system("./memsim gcc.trace 4 lru quiet")
os.system('rm output_file.txt')
os.system("touch output_file.txt")

files = ['gcc.trace', 'bzip.trace', 'sixpack.trace', 'swim.trace']
frame_sizes = [i for i in range(2, 102, 2)]
replacement_type = ['lru', 'clock', 'rand']

# import pandas as pd
# from IPython.utils.capture import capture_output
def capture(f):
    """
    Decorator to capture standard output
    """
    def captured(*args, **kwargs):
        import sys
        from io import StringIO

        # setup the environment
        backup = sys.stdout

        try:
            sys.stdout = StringIO()     # capture output
            f(*args, **kwargs)
            out = sys.stdout.getvalue() # release output
        finally:
            sys.stdout.close()  # close the stream 
            sys.stdout = backup # restore original stdout

        return out # captured output wrapped in a string

    return captured

@capture
def get_out(file, frame, rep):
    return os.system("./memsim {} {} {} quiet".format(file, frame, rep)) #  >> output_file.txt

for file in files:
    # for each file I want to run the memsim code and get the stdout and throw it into the output_file
    for frame in frame_sizes:
        for rep in replacement_type:
            os.system("echo 'file: {} frames: {} replacement: {} ' >> output_file.txt".format(file, frame, rep))
            os.system("./memsim {} {} {} quiet >> output_file.txt".format(file, frame, rep)) #  >> output_file.txt



