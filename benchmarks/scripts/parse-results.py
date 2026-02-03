import pathlib
import sys
import re

benchmarks_folder = "benchmarks/dodo"
benchmarks_suffix = "json"
results_folder = pathlib.Path(__file__).parent / "results"

def line_to_min_sec(s):
    s1 = s.split("m")
    mins = int(''.join(c for c in s1[0] if c.isdigit()))
    secs = float(s1[1].replace("s", "").replace(",", "."))
    return (mins, secs)

def extract_name(line):
    # name_match = re.search(r"[^\/\s]*\.json", line) # problem: some benchmarks are named "main.json" with the folder giving the actual name...
    name_match = re.search(benchmarks_folder + r".*/([\S]*)\." + benchmarks_suffix, line)
    if name_match:
        return name_match.group(1)
    else:
        return None

def parse_list(l):
    return [i.strip() for i in l.strip("[]").split(",")]

def parse_raw_bench(f):
    results = []

    lines = [l.strip() for l in f.readlines()]
    instance = None
    # iteration = None
    for i in range(len(lines)):
        l = lines[i]

        # extract name of instance (detects lines of the form benchmarks_folder + sth)
        if benchmarks_folder in l and ("." + benchmarks_suffix) in l:
            name = extract_name(l)
            if name == None:
                print("Could not find benchmark name in line " + str(i) + ":\n" + l)
                sys.exit(1)
            else:
                if instance == None or (instance != None and "name" in instance and instance["name"] != name):
                    # new instance
                    if instance != None:
                        if "output" not in instance:
                            instance["output"] = "OOM"
                        results.append(instance)
                    instance = {}
                instance["name"] = name
        # extract information of benchmarks
        if match := re.search(r"using ([\S]*) interpretation", l):
            instance["interpretation"] = match.group(1)
        if match := re.search("Result: (.*)", l):
            instance["output"] = [int(i) for i in parse_list(match.group(1))]
            instance["solved_num"] = sum(instance["output"])
            instance["properties_num"] = len(instance["output"])
        elif "the following properties could be separated from the initial configurations" in l:
            instance["solved"] = parse_list(lines[i + 1].strip())
        elif "the following properties could not be separated from the initial configurations" in l:
            instance["unsolved"] = parse_list(lines[i + 1].strip())
        elif l.startswith("user"):
            # determine time instance needed
            (m1, s1) = line_to_min_sec(lines[i])
            (m2, s2) = line_to_min_sec(lines[i+1])
            m = m1 + m2
            s = s1 + s2
            if s >= 60.0:
                m = m + 1
                s = s - 60.0
            if "output" not in instance:
                instance["output"] = "TIMEOUT"
            else:
                instance["time"] = "{:.3f}".format(60*m + s)
        elif "Killed" in l:
            instance["output"] = "OOM"

    if instance != None:
        if "output" not in instance:
            instance["output"] = "OOM"
        results.append(instance)

    return results

# take a list of dicts as input, extract keys, and build csv from it (with NA if a key is missing in a dict)
def print_as_csv(dicts, filename, header=True):
    # collect keys
    keys = set()
    for d in dicts:
        for k in d:
            keys.add(k)
    keys = list(keys)
    # write csv
    if header:
        filename.write(",".join(keys) + "\n")
    for d in dicts:
        d_keys = []
        for k in keys:
            if k in d:
                d_keys.append(str(d[k]))
            else:
                d_keys.append("NA")
        filename.write(",".join(d_keys) + "\n")

def raw_results_path(interpretation):
    return results_folder / "raw" / (interpretation + ".txt")

csv_output_path = results_folder / "processed.csv"

results = []
for interpretation in ["flow", "siphon", "trap"]:
    # read results from raw file
    with open(raw_results_path(interpretation), "r") as f:
        results_interpretation = parse_raw_bench(f)
        results.append(results_interpretation)

def make_csv_ready(results):
    return [{k: v for k, v in result.items() if k not in ["solved", "unsolved", "output"]} for result in results]

# write results to csv files
with open(csv_output_path, "w") as f:
    need_header = True
    for r in results:
        print_as_csv(make_csv_ready(r), f, need_header)
        need_header = False

### dodo parsing ###

def parse_dodo(filename):
    results = []

    lines = [l.strip() for l in filename.readlines()]
    instance = None
    skipinstance = False
    # iteration = None
    for i in range(len(lines)):
        l = lines[i]

        # extract name of instance
        if "\"call\"" in l:
            name = re.search(r"/([\S]*)\.json", l).group(1)
            if name == None:
                print("Could not find benchmark name in line " + str(i) + ":\n" + l)
                sys.exit(1)
            else:
                if instance != None and not skipinstance:
                    if "output" not in instance:
                        instance["output"] = "OOM"
                    results.append(instance)
                instance = {}
                skipinstance = False
                instance["name"] = name
                alg = re.search(r"(learn|oneshot|adaptive)[\s]([\S]*)[\s][\S]*\.json ([\S]*)\",$", l).group(1)
                if alg != "learn": # TODO later: probably compare with oneshot
                    skipinstance = True
                itype = re.search(r"(learn|oneshot|adaptive)[\s]([\S]*)[\s][\S]*\.json ([\S]*)\",$", l).group(2) # group(1): "learn" or "adaptive" or "oneshot"
                propertyName = re.search(r"(learn|oneshot|adaptive)[\s]([\S]*)[\s][\S]*\.json ([\S]*)\",$", l).group(3)
                instance["interpretation"] = itype
                instance["propertyName"] = propertyName
                if itype not in ["s", "t", "f"]:
                    skipinstance = True

        # extract information of benchmarks
        if match := re.search(r"\"result\": \"([\S]*)\",", l):
            instance["output"] = match.group(1)
        if match := re.search(r"\"time\(ms\)\": ([0-9]*),", l):
            instance["time"] = int(match.group(1))

    if instance != None and not skipinstance:
        if "output" not in instance:
            instance["output"] = "OOM"
        results.append(instance)

    return results

# if dodo results are present: parse dodo results
dodo_path = results_folder / "raw" / "dodo.txt"

if dodo_path.exists():
    with open(dodo_path) as dodo_f:
        dodo_results = parse_dodo(dodo_f)
        with open(results_folder / "dodo_processed.csv", "w") as f:
            print_as_csv(dodo_results, f)
