rsync -avz --exclude build --exclude workspace/pro --exclude '.git' /home/likp/work/Track-trt/ root@192.168.9.118:/root/Track-trt/
ssh root@192.168.9.118 "cd /root/Track-trt/build && make -j4
