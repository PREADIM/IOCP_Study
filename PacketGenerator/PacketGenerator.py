import argparse # C++ main 함수에 인자를 전달할수있는 것처럼(char* ch, int args[]) 실핼할때 인자를 편하게 전달하게 해주는 라이브러리.
import jinja2
import ProtoParser


def main():

    # 파이썬은 C++처럼 타입을 정하지 않아도 알아서 문맥상 알맞게 정해짐. auto문 느낌
    # add_argument에서 첫번째는 변수이름, type은 무슨 타입인지, 세번째는 기본 값이다.
    arg_parser = argparse.ArgumentParser(description = 'PacketGenerator')
    arg_parser.add_argument('--path', type=str, default = 'C:/Users/sngbi/source/repos/GameServer/Common/Protobuf/bin/Protocol.proto', help= 'proto path')
    arg_parser.add_argument('--output', type=str, default = 'TestPacketHandler', help= 'output file')
    arg_parser.add_argument('--recv', type=str, default = 'C_', help= 'recv convention')
    arg_parser.add_argument('--send', type=str, default = 'S_', help= 'send convention')
    args = arg_parser.parse_args() # 이 함수를 호출하면 위의 내용이 파싱되어서 사용할수 있게됨


    parser = ProtoParser.ProtoParser(1000, args.recv, args.send)
    parser.parse_proto(args.path)

    #jinja2
    file_loader = jinja2.FileSystemLoader('Templates')
    env = jinja2.Environment(loader = file_loader)

    template = env.get_template('PacketHandler.h')
    output = template.render(parser = parser, output = args.output) # 왼쪽이 안에서 사용할 이름 오른쪽이 넘겨준 변수 이름


    f = open(args.output+'.h', 'w+') # output에 +.h를 붙여서 파일이름을 만들어 줄 것이고 w+ 덮어쓰기.
    f.write(output)
    f.close()

    print(output)
    return



if __name__ =='__main__':
    main()
