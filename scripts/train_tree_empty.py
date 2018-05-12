import os
import sys


sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import dynet_config
dynet_config.set(random_seed=42, mem=512, autobatch=False, requested_gpus=0)

from max_sub_tree.mstlstm_empty import MaxSubTreeWithEmptyParser


op = {
    "epochs": 20,
    "lstmdims": 150,
    "lstmlayers": 2,
    "activation": "relu",
    "pembedding": 50,
}

home = os.path.expanduser("~")

outdir_prefix = f"{home}/Development/nullnode/20180203/"
data_dir = f"{home}/Development/large-data/emptynode-data/"
trainer = MaxSubTreeWithEmptyParser.get_training_scheduler()

extrn = {
    "en": data_dir + "../sskip.100.vectors",
    "cn": data_dir + "../sample.win7.vector"
}

for lang in ("cn",):
    for enable_2nd in (True,):
        op["enable-2nd"] = enable_2nd
        op["extrn"] = extrn[lang]

        op["train"] = data_dir + f"{lang}.trn.tagged.nept"
        op["dev"] = [data_dir + f"{lang}.dev.nept", data_dir + f"{lang}.tst.nept"]
        trainer.add_options(f"{lang}_label_noempty_{enable_2nd}+label", op, outdir_prefix)
 
trainer.run_parallel()
