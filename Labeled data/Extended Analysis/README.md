This directory includes the files of the manual assessment results in Discussion.

We manually assess whether the test inputs we generated are qualified.

```CAT.txt```: the labeled test inputs for CAT.

```TransRepair.txt```: the labeled test inputs for TransRepair.

```SIT.txt```: the labeled test inputs for SIT.

```PatInv.txt```: the labeled test inputs for PatInv.

In these files, each test is decomposed into 4 lines.

Line 1: the English sentence before and after replacement/mutation

Line 2: the Chinese sentence before and after replacement/mutation

Line 3: the results of whether the test reveals a bug reported automatically. (```True``` for a bug, ```False``` for not a bug)

Line 4: the results of whether the test reveals a bug inspected by human. (```True``` for a bug, ```False``` for not a bug)


```Purity.txt```: the labeled test inputs for Purity.

In this file, each test is decomposed into 6 lines.

Line 1: the English sentence before mutation

Line 2: the Chinese sentence before mutation

Line 3: the English sentence after mutation

Line 4: the Chinese sentence after mutation

Line 5: the results of whether the test reveals a bug reported automatically. (```True``` for a bug, ```False``` for not a bug)

Line 6: the results of whether the test reveals a bug inspected by human. (```True``` for a bug, ```False``` for not a bug)
