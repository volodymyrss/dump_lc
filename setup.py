from distutils.core import setup

setup(
        name='dump_ihklc',
        version='1.0',
        py_modules= ['dump_ihklc'],
        package_data     = {
            "": [
                "*.txt",
                "*.md",
                "*.rst",
                "*.py"
                ]
            },
        license='Creative Commons Attribution-Noncommercial-Share Alike license',
        long_description=open('README.md').read(),
        )

