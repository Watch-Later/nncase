from typing import Any, List, BinaryIO

import numpy


class CompileOptions:
    benchmark_only: bool
    dump_asm: bool
    dump_dir: Path
    dump_ir: bool
    swapRB: bool
    input_range: List[float]
    input_shape: List[int]
    input_type: str
    is_fpga: bool
    mean: List[float]
    std: List[float]
    output_type: str
    preprocess: bool
    quant_type: str
    target: str
    w_quant_type: str
    use_mse_quant_w: bool
    split_w_to_act: bool
    input_layout: str
    output_layout: str
    letterbox_value: float
    def __init__(self) -> None: ...


class Compiler:
    def __init__(self, compile_options: CompileOptions) -> None: ...
    def compile(self) -> None: ...
    def create_evaluator(self, stage: int) -> GraphEvaluator: ...
    def gencode(self, stream: BinaryIO) -> None: ...
    def gencode_tobytes(self) -> bytes: ...
    def import_caffe(self, model: bytes, prototxt: bytes) -> None: ...
    def import_onnx(self, model: bytes, options: ImportOptions) -> None: ...
    def import_tflite(self, model: bytes, options: ImportOptions) -> None: ...
    def use_ptq(self, ptq_dataset_options: PTQTensorOptions) -> None: ...


class GraphEvaluator:
    def __init__(self) -> None: ...
    def get_input_tensor(self, index: int) -> Any: ...
    def get_output_tensor(self, index: int) -> Any: ...
    def run(self) -> None: ...
    @property
    def outputs_size(self) -> int: ...


class ImportOptions:
    def __init__(self) -> None: ...


class MemoryRange:
    dtype: dtype
    location: int
    size: int
    start: int
    def __init__(self) -> None: ...


class PTQTensorOptions:
    calibrate_method: str
    input_mean: float
    input_std: float
    samples_count: int
    def __init__(self) -> None: ...
    def set_tensor_data(self, bytes: bytes) -> None: ...


class Path:
    def __init__(self, path: str) -> None: ...


class RuntimeTensor:
    def __init__(self) -> None: ...
    def copy_to(self, to: RuntimeTensor) -> None: ...
    def from_numpy(self, arr: numpy.ndarray) -> Any: ...
    def to_numpy(self) -> numpy.ndarray: ...
    @property
    def dtype(self) -> dtype: ...
    @property
    def shape(self) -> List[int]: ...


class Simulator:
    def __init__(self) -> None: ...
    def get_input_desc(self, index: int) -> MemoryRange: ...
    def get_input_tensor(self, index: int) -> RuntimeTensor: ...
    def get_output_desc(self, index: int) -> MemoryRange: ...
    def get_output_tensor(self, index: int) -> RuntimeTensor: ...
    def load_model(self, model: bytes) -> None: ...
    def run(self) -> None: ...
    def set_input_tensor(self, index: int, tensor: RuntimeTensor) -> None: ...
    def set_output_tensor(self, index: int, tensor: RuntimeTensor) -> None: ...
    @property
    def inputs_size(self) -> int: ...
    @property
    def outputs_size(self) -> int: ...


def test_target(target: str) -> bool: ...
