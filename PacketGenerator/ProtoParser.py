

class ProtoParser():
    def __init__(self, start_id, recv_prefix, send_prefix):
        self.recv_pkt = [] # 수신 패킷 목록
        self.send_pkt = [] # 송신 패킷 목록
        self.total_pkt = [] # 모든 패킷 목록
        self.start_id = start_id
        self.id = start_id
        self.recv_prefix = recv_prefix
        self.send_prefix = send_prefix


    def parse_proto(self, path): # 이것이 파이썬의 함수 생성 방법
        f = open(path, 'r')
        lines = f.readlines()

        for line in lines: # for-each문
            if line.startswith('message') == False:
                continue


            pkt_name = line.split()[1].upper()
            if pkt_name.startswith(self.recv_prefix):
                self.recv_pkt.append(Packet(pkt_name, self.id)) #append 는 vector push_back 같은것
            elif pkt_name.startswith(self.send_prefix):
                self.send_pkt.append(Packet(pkt_name, self.id))
            else:
                continue


            self.total_pkt.append(Packet(pkt_name, self.id)) # 배열에 패킷 이름과 숫자 저장
            self.id += 1 # 패킷 번호 증가
 

        f.close()


class Packet:
    def __init__(self, name, id):
        self.name = name
        self.id = id





                    

