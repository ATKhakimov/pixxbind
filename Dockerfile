FROM python:3.10-slim

RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential cmake ninja-build \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN pip install --upgrade pip
RUN pip install build pytest scikit-build-core pybind11 numpy
RUN rm -rf dist && python -m build
RUN pip install dist/*linux*.whl

CMD ["pytest", "-q", "tests"]
