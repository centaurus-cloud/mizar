from setuptools import setup, find_packages

setup(
    name='mizar',
    version='0.1.0',
    packages=find_packages(include=['mizar', 'mizar.*']),
    entry_points={
        'console_scripts': [
            'mizard=mizar.daemon.app:main'
        ]
    },
    install_requires=[
        'PyYAML',
        'kopf',
        'netaddr',
        'ipaddress',
        'pyroute2',
        'rpyc',
        'kubernetes==11.0.0',
        'luigi==2.8.12',
        'grpcio',
        'protobuf',
        'fs'
    ]
)
