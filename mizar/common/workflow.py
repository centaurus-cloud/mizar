# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Sherif Abdelwahab <@zasherif>
#          Phu Tran          <@phudtran>

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

from __future__ import annotations
import luigi
from abc import ABC, abstractmethod
from mizar.common.wf_param import *


class WorkflowTask(luigi.Task):
    is_complete = False
    temporary_error_status = False
    permanent_error_status = False
    error_message = ""
    delay = 5
    param = HandlerParam()

    def finalize(self):
        self.is_complete = True
        self.temporary_error_status = False
        self.permanent_error_status = False
        self.error_message = ""

    def raise_temporary_error(self, error_message):
        self.temporary_error_status = True
        self.error_message = error_message
        raise Exception(self.error_message)

    def raise_permanent_error(self, error_message):
        self.permanent_error_status = True
        self.error_message = error_message
        raise Exception(self.error_message)

    def set_retry_delay(self, delay):
        self.delay = delay

    @property
    def temporary_error(self):
        return self.temporary_error_status

    @property
    def permanent_error(self):
        return self.permanent_error_status

    @property
    def error(self):
        return self.error_message

    @property
    def retry_delay(self):
        return self.delay

    def complete(self):
        return self.is_complete

    def requires(self):
        return []

    def run(self):
        self.finalize()


class WorkflowFactory(ABC):

    @abstractmethod
    def VpcOperatorStart(self, param):
        pass

    @abstractmethod
    def VpcCreate(self, param):
        pass

    @abstractmethod
    def VpcProvisioned(self, param):
        pass

    @abstractmethod
    def VpcDelete(self, param):
        pass

    @abstractmethod
    def DividerOperatorStart(self, param):
        pass

    @abstractmethod
    def DividerCreate(self, param):
        pass

    @abstractmethod
    def DividerProvisioned(self, param):
        pass

    @abstractmethod
    def DividerDelete(self, param):
        pass

    @abstractmethod
    def DropletOperatorStart(self, param):
        pass

    @abstractmethod
    def DropletCreate(self, param):
        pass

    @abstractmethod
    def DropletProvisioned(self, param):
        pass

    @abstractmethod
    def DropletDelete(self, param):
        pass

    @abstractmethod
    def NetOperatorStart(self, param):
        pass

    @abstractmethod
    def NetCreate(self, param):
        pass

    @abstractmethod
    def NetProvisioned(self, param):
        pass

    @abstractmethod
    def NetDelete(self, param):
        pass

    @abstractmethod
    def BouncerOperatorStart(self, param):
        pass

    @abstractmethod
    def BouncerCreate(self, param):
        pass

    @abstractmethod
    def BouncerProvisioned(self, param):
        pass

    @abstractmethod
    def BouncerDelete(self, param):
        pass

    @abstractmethod
    def k8sServiceCreate(self, param):
        pass

    @abstractmethod
    def k8sEndpointsUpdate(self, param):
        pass

    @abstractmethod
    def k8sEndpointsDelete(self, param):
        pass

    @abstractmethod
    def k8sDropletCreate(self, param):
        pass

    @abstractmethod
    def k8sPodCreate(self, param):
        pass

    @abstractmethod
    def k8sPodDelete(self, param):
        pass
