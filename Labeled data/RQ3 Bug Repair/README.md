This directory includes the files of the manual assessment results in RQ3.

We manually inspect the changed translations before and after repairing.

We compared two dimensions: 

1) the translation consistency before and after repair;

2) the quality of the changed translations.

```CAT.txt```: the changed translation of CAT.

```TransRepair.txt```: the changed translation of TransRepair.

In these files, each changed translation is decomposed into 28 lines.

  Line 2: the original sentence
 
  Line 4: the translation of the original sentence before repairing
  
  Line 6: the translation of the original sentence after repairing
  
  Line 8: the differences between the original sentence before and after repairing
  
  Line 9: the results of whether the translation improves inspected by human.
 
  Line 12: the mutant
 
  Line 14: the translation of the mutant before repairing
  
  Line 16: the translation of the mutant after repairing
  
  Line 18: the differences between the mutant before and after repairing
  
  Line 19: the results of whether the translation improves inspected by human

  Line 23: the differences between the original sentence and mutant
  
  Line 25: the differences between the translation of original sentence and mutant before repairing
  
  Line 27: the differences between the translation of original sentence and mutant after repairing
  
  Line 28: the results of whether the consistency improves inspected by human
