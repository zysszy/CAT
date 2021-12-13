This directory includes the files of the manual assessment results in RQ2.

We manually assess whether the test inputs we generated are qualified.

```CAT.txt```: the labeled test inputs for CAT.

```TransRepair.txt```: the labeled test inputs for TransRepair.

In these files, each test is decomposed into 4 lines.

Line 1: the English sentence before and after replacement

Line 2: the Chine sentence before and after replacement

Line 3: the results of whether the test reveals a bug reported automatically by the LCS metric. (```True``` for inconsistent bug, ```False``` for consistent)

Line 4: the results of whether the test reveals a bug inspected by human. (```True``` for inconsistent bug, ```False``` for consistent)
