FROM debian

RUN apt update
RUN apt install -y git build-essential libjansson-dev zlib1g-dev libssl-dev

COPY Miniconda3.sh /tmp
RUN bash -c 'yes yes | bash /tmp/Miniconda3.sh'
RUN bash -c 'source /root/.bashrc ; conda install -y conda-build'

COPY . /tmp
WORKDIR /tmp
RUN bash -c 'source /root/.bashrc ; conda build .'
RUN bash -c 'source /root/.bashrc ; conda install -y --use-local dime2'

ENTRYPOINT []
