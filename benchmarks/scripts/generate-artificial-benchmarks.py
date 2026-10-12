import json
import pathlib

output_folder = pathlib.Path(__file__).parent.parent / "artificial"

def gen_permutation_benchmark(n):
    alphabet = [str(i) for i in range(n)]

    # initial config: "identity" permutation
    initial = {
        "initialStates": [0],
        "alphabet": alphabet,
        "transitions": [{"letter": str(i), "source": i, "target": [i + 1]} for i in range(n)],
        "states": list(range(n + 1)),
        "accepting": [n],
    }

    # transducer: can swap two adjacent letters
    transducer_states = [0, 1]
    transducer_transitions = [{"letter": f"[{str(i)},{str(i)}]", "source": l, "target": [l]} for i in range(n) for l in [0, 1]] # id on first and last state
    for i in range(n):
        for j in range(n):
            if i != j:
                new_state = len(transducer_states)
                transducer_states.append(new_state)
                transducer_transitions.append({"letter": f"[{str(i)},{str(j)}]", "source": 0, "target": [new_state]})
                transducer_transitions.append({"letter": f"[{str(j)},{str(i)}]", "source": new_state, "target": [1]})
    transducer = {
        "initialStates": [0],
        "transitions": transducer_transitions,
        "states": transducer_states,
        "accepting": [0, 1],
    }

    # property 1: no letter appears twice -> unsafe: a letter appears twice
    property_unique = {
        "initialStates": [0],
        "alphabet": alphabet,
        "transitions": [{"letter": str(i), "source": 0, "target": [0, i + 1]} for i in range(n)] +
                [{"letter": str(j), "source": i + 1, "target": [i + 1] if i != j else [i + 1, n + 1]} for i in range(n) for j in range(n)] +
                [{"letter": str(i), "source": n + 1, "target": [n + 1]} for i in range(n)],
        "states": list(range(n + 2)),
        "accepting": [n + 1],
    }

    # property 2: every letter appears once -> unsafe: a letter does not appear
    property_exist = {
        "initialStates": [0],
        "alphabet": alphabet,
        "transitions": [{"letter": str(i), "source": 0, "target": [j + 1 for j in range(n) if j != i]} for i in range(n)] +
                [{"letter": str(i), "source": j + 1, "target": [j + 1]} for i in range(n) for j in range(n) if i != j],
        "states": list(range(n + 1)),
        "accepting": list(range(n + 1)),
    }

    benchmark = {
        "transducer": transducer,
        "initial": initial,
        "properties": {
            "unique": property_unique,
            "exist": property_exist,
        },
    }

    output_file = output_folder / f"permutations_{n}.json"
    output_folder.mkdir(parents=True, exist_ok=True)
    with open(output_file, "w") as f:
        json.dump(benchmark, f, indent=2)

if __name__ == "__main__":
    gen_permutation_benchmark(3)
