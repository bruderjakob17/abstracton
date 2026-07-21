# Comparison of abstracton (mata) with dodo (AutomataLib)

A simple example of how invocation differs:

abstracton:
1. Build using `make`
2. Run `solve_dodo`, e.g.

```
time ./solve_dodo dodo/MESI.json -p modifiedmodified -i f -v 2
```

dodo:
1. Build artifact in `dodo_experiments` (IntelliJ: `Build > Build Artifact... > dodo:jar > Build)
2. Run jar artifact with corretto 19.0.2, e.g.

```
~/.jdks/corretto-19.0.2/bin/java -Xmx10G -jar out/artifacts/dodo_jar/dodo.jar oneshot f benchmark/MESI.json modifiedmodified
```

In order to print the learned automaton of an experiment in LearnLib, use

```
GraphDOT.write((CompactDFA<Integer>) experiment.getFinalHypothesis(), System.out);
```

In order to print any automaton in AutomataLib, use

```
GraphDOT.write(automaton, System.out);
```

## what does solve_dodo_old do?
-> was built on accident by swapping the H, M labels in SetInterpretations. Result seems to be the same, but with qualitatively different running time (to be confirmed by experiments).
