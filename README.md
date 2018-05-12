# Empty Parser

## Recommended Environment
- Linux
- Python 3.6
- GCC 7.0

## Data Preparation
Parpare the data in 4-column format. Each for form, POSTag, head and relation. Word with POSTag "EMCAT" will be regarded as empty category.

Example:

```
据悉    AD      13      ADV
，      PU      13      UNK
中国    NR      4       NMOD
进出口  NN      4       NMOD
银行    NN      13      SBJ
近期    NT      13      TMP
还      AD      13      ADV
将      AD      13      ADV
相继    AD      13      ADV
与      P       13      ADV
其他    DT      12      DMOD
商业    NN      12      NMOD
银行    NN      9       OBJ
签署    VV      -1      ROOT
*OP*    EMCAT   17      ADV
*T*     EMCAT   16      SBJ
类似    VA      17      COMP
的      DEC     20      RELC
委托    NN      20      NMOD
代理    NN      20      NMOD
协议    NN      13      COMP
。      PU      13      UNK
```

### install python dependency
Run ```pip3 install -r requirements.txt```

### Train a Parser
Modify paths in ```scripts/train_tree_empty.py``` and run it.

### Predict with trained model
Run ```main.py mst+empty predict --model <model path> --test <test file> --output <output file> --eval```
