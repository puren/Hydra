# Copyright 2022, Massachusetts Institute of Technology.
# All Rights Reserved
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#  1. Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#
#  2. Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Research was sponsored by the United States Air Force Research Laboratory and
# the United States Air Force Artificial Intelligence Accelerator and was
# accomplished under Cooperative Agreement Number FA8750-19-2-1000. The views
# and conclusions contained in this document are those of the authors and should
# not be interpreted as representing the official policies, either expressed or
# implied, of the United States Air Force or the U.S. Government. The U.S.
# Government is authorized to reproduce and distribute reprints for Government
# purposes notwithstanding any copyright notation herein.
#
#
"""Module containing python utilites for loading a valid config."""
from hydra_python._hydra_bindings import PythonConfig
from typing import Dict, Any
import pathlib
import click
import yaml


DEFAULT_CONFIGS = [
    ("frontend", "frontend_config.yaml"),
    ("backend", "backend_config.yaml"),
    ("reconstruction", "reconstruction_config.yaml"),
]


def get_config_path(dataset=None):
    """Return config directory for this package."""
    package_path = pathlib.Path(__file__).absolute().parent
    install_path = package_path / "config"
    if install_path.exists():
        return install_path

    devel_path = package_path.parent.parent.parent / "config"
    if devel_path.exists():
        return devel_path

    click.secho("failed to find config directory of package!", fg="red")
    raise RuntimeError("invalid config path")


def load_configs(
    dataset_name: str,
    camera_info: Dict[str, Any],
    labelspace_name: str = "ade20k_mp3d",
    bounding_box_type: str = "AABB",
):
    """
    Load various configs to construct the Hydra pipeline.

    dataset_name: Dataset name to load config from
    camera_info: Config dictionary for camera
    labelspace_name: Labelspace name to use
    bounding_box_type: Type of bounding box to use

    Returns:
        (Optional[PythonConfig]) Pipline config or none if invalid
    """
    config_path = get_config_path()
    dataset_path = config_path / dataset_name
    if not dataset_path.exists():
        click.secho(f"invalid dataset path: {dataset_path}", fg="red")
        return None

    labelspace_path = (
        config_path / "label_spaces" / f"{labelspace_name}_label_space.yaml"
    )
    if not labelspace_path.exists():
        click.secho(f"invalid labelspace path: {labelspace_path}", fg="red")
        return None

    configs = PythonConfig()
    for ns, config_name in DEFAULT_CONFIGS:
        dataset_config = dataset_path / config_name
        if dataset_config.exists():
            configs.add_file(dataset_config, config_ns=ns)

    configs.add_file(labelspace_path)

    pipeline = {
        "frontend": {"type": "FrontendModule"},
        "backend": {"type": "BackendModule"},
        "reconstruction": {
            "type": "ReconstructionModule",
            "sensor": {
                "type": "camera",
                "min_range": 0.1,
                "max_range": 5.0,
                "extrinsics": {"type": "identity"},
            },
        },
    }
    pipeline["reconstruction"]["sensor"].update(camera_info)
    configs.add_yaml(yaml.dump(pipeline))

    overrides = {
        "frontend": {"objects": {"bounding_box_type": bounding_box_type}},
        "lcd": {"lcd_use_bow_vectors": False},
        "reconstruction": {
            "show_stats": False,
            "pose_graphs": {"make_pose_graph": True},
        },
    }
    configs.add_yaml(yaml.dump(overrides))

    return configs
