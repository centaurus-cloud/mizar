#!/bin/bash

# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Hong Chang        <@Hong-Chang>
#          Sherif Abdelwahab <@zasherif>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:The above copyright
# notice and this permission notice shall be included in all copies or
# substantial portions of the Software.THE SOFTWARE IS PROVIDED "AS IS",
# WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
# FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.

is_mizar_production=${1:-0}
# Environments: k8s_kind, k8s_gcp, k8s_aws, 
#               arktos_up, arktos_gcp, arktos_aws, 
#               arktos_multi_master, arktos_with_kubemark, 
mizar_environment=${2:-"arktos_up"}

# Make sure script files are present and execute location is correct.
if [[ ! -f install/deploy_steps.sh ]]; then
    if [[ $is_mizar_production == 1 ]]; then
        # TODO: When deploy Mizar production, we support one-file installation that user only grab this file then complete the deployment.
        #       So here we will download necessary script/files. 
        echo "[deploy-mizar] Error: Not implemented for TODO item." 1>&2
        exit 1
    else    
        echo "[deploy-mizar] Error: Need to execute the script from Mizar home directory. current DIR is $(pwd)." 1>&2
        exit 1
    fi
fi
source install/common.sh
source install/deploy_steps.sh

# Source different adaptor file for different environment. In this way we use same function but do different thing for different environment. 
common:source_environment_adaptor $mizar_environment

# Execute deploy step by step
deploy_steps:binaries_ready $is_mizar_production
deploy_steps:environment_ready $is_mizar_production
deploy_steps:deploy_mizar $is_mizar_production
deploy_steps:sanity_check $is_mizar_production
