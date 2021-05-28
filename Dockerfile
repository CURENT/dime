FROM debian

RUN apt update
RUN apt install -y wget build-essential

RUN wget http://repo.anaconda.com/miniconda/Miniconda3-py39_4.9.2-Linux-x86_64.sh -O /tmp/miniconda.sh
RUN bash -c 'test `sha256sum /tmp/miniconda.sh | cut -f 1 -d " "` = 536817d1b14cb1ada88900f5be51ce0a5e042bae178b5550e62f61e223deae7c'
RUN bash /tmp/miniconda.sh -b -p /root/miniconda
RUN bash -c 'source /root/miniconda/etc/profile.d/conda.sh ; conda init'
RUN bash -c 'source /root/miniconda/etc/profile.d/conda.sh ; conda install -y conda-build'
#RUN bash -c 'source /root/miniconda/etc/profile.d/conda.sh ; conda install -c conda-forge jansson'
RUN bash -c 'source /root/miniconda/etc/profile.d/conda.sh ; conda config --add channels conda-forge'

COPY . /tmp/hello
WORKDIR /tmp/hello
RUN rm -rf .git
RUN rm -f .gitignore
RUN bash -c 'source /root/miniconda/etc/profile.d/conda.sh ; conda build .'
RUN bash -c 'source /root/miniconda/etc/profile.d/conda.sh ; conda install -y --use-local dime2'

ENTRYPOINT []
