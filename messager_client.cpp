
/*Author:Bowen Li
 Course: CPDP
 Program:Client.cpp
 */

/**********HEADER FILE***********************/
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <typeinfo>
#include <ifaddrs.h>
#include <sys/select.h>
#include<signal.h>
using namespace std;
/*********GLOABLE VARIVALES***********************/
int client_socket_no;
int cc_listen_socket;
fd_set fds;
/**********CLASS DECLARATION***********************/
class User{
public:
    string name;
    string password;
    int socket;
    string ip;
    string port;
    vector<string> friend_list;
};
User current_user;
/**********FUNCTION DELCARATION***********************/
string read_config_info();
vector<string> split(const string& str, const string& delim);
int init_socket();
void print_1st_menu();
void login_1st_menu();
int regist();
int login();
int show_online_friends(string name);
void exit_socket();
void print_2nd_menu();
void login_2nd_menu();
int command_2nd_handler(string option);
int invite(string name, string message);
int accept(string name, string message);
int logout();
string getIPAddress();
int init_listen_socket();
int message(string to_name,string information);
void sig_int_handler(int sig_num);
/**********MAIN  FUNCTION***********************/
int main(int argc, char *argv[]){
    
    signal(SIGINT,sig_int_handler);
    client_socket_no=init_socket();
    cout<<"Creat socket:"<<client_socket_no<<endl;
    if(client_socket_no < 0){
        cout<<"Initialize socket failed!!"<<endl;
        return 1;
    }else{
        cout<<"***************************WELCOME!***************************"<<endl;
        print_1st_menu();
        login_1st_menu();
    }
    
    
    return 0;
}

/************Read configration information************************/
string read_config_info(){
    string server_host;
    string server_port;
    string str;
    vector<string>config_result;
    ifstream config_info("./client_file/configration_file");
    if(!config_info) {
        cout << "Cannot open input file.\n";
        exit(1);
    }
    
    while(config_info) {
        getline(config_info,str);
        if(str!=""){
            config_result=split(str, ":");
            if(config_result[0]!="servhost")
                server_host=config_result[1];
            else if(config_result[0]!="servpost")
                server_port=config_result[1];
        }
    }
    
    return server_host+';'+server_port;
}
/************Split function************************/
vector<string> split(const string& str, const string& delim) {
    vector<string> res;
    if("" == str) return res;
    char * strs = new char[str.length() + 1] ;
    strcpy(strs, str.c_str());
    char * d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());
    char *p = strtok(strs, d);
    while(p) {
        string s = p;
        res.push_back(s);
        p = strtok(NULL, d);
    }
    
    return res;
}
/************Init socket on clients************************/
int init_socket(){
    string str;
    str=read_config_info();
    vector<string>config_info;
    config_info=split(str, ";");
    int socket_client;
    struct sockaddr_in addr;
    if ((socket_client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror(": Can't get socket");
        exit(1);
    }
    cout<<"Creat client socket:"<<socket_client<<endl;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    int port;
    port=stoi(config_info[0]);
    char ip[100];
    strcpy(ip,config_info[1].c_str());
    cout<<"servhost:"<<ip<<endl;
    cout<<"servport:"<<port<<endl;
    cout<<"client socket no:"<<socket_client<<endl;
    addr.sin_family = AF_INET;
    addr.sin_port =port;
    addr.sin_addr.s_addr = inet_addr(ip);
    
    if (connect(socket_client, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror(":connect");
        exit(1);
    }
    return socket_client;
}
/************Print out 1st-login menu************************/
void print_1st_menu(){
    cout<<"--------------menu----------------\n";
    cout<<"\t r:register\n";
    cout<<"\t l:login\n";
    cout<<"\t exit:exit\n";
    cout<<"--------------menu----------------\n";
}
/************Login 1st menu ************************/
void login_1st_menu(){
    string option;
    cout<<"Your choice:"<<endl;
    cin>>option;
    if(option=="r"){
        cout<<"Register"<<endl;
        regist();
    }
    else if(option=="l"){
        cout<<"Login"<<endl;
        login();
    }
    else if(option=="exit"){
        cout<<"Exit"<<endl;
        exit_socket();
    }
    else {
        cout<<"Wrong option"<<endl;
        print_1st_menu();
        login_1st_menu();
    }
    
}
/************Regist Funtion************************/
int regist(){
    
    cout<<"Hi, welcome to regisr a new username:"<<endl;
    string name;
    cout<<"Please input your Username:"<<endl;
    cin>>name;
    string password;
    cout<<"Please input your Password:"<<endl;
    cin>>password;
    string send_info="r|"+name+'|'+password+'|';
    
    
    int sending_re = (int)::send(client_socket_no,send_info.c_str(),(int)send_info.size(),0);
    cout<<"Client message length:"<<sending_re<<endl;
    
    if(sending_re > 0)
        fprintf(stderr, "\tSending message success, bytes: %d\n",sending_re);
    else{
        fprintf(stderr, "Sending message error\n");
    }
    
    cout<<"Wating for the server response......\n"<<endl;
    int socket_length = 0;
    char receive_buff[256];
    socket_length = (int)::recv(client_socket_no, receive_buff, 255, 0);
    string receive_status=receive_buff;
    if(socket_length > 0){
        fprintf(stderr, "\tReceive message success, bytes: %d\n",sending_re);
        if(receive_status.substr(0,socket_length)=="500"){
            cout<<"Username already exists, please try a new name!"<<endl;
            regist();
        }else if(receive_status.substr(0,socket_length)=="200"){
            cout<<"Regist successfully!"<<endl;
            print_1st_menu();
            login_1st_menu();
        }
    }
    else{
        fprintf(stderr, "Receive message error\n");
    }
    
    
    return 0;
}
/************Login in Function************************/
int login(){
    cout<<"Hi, welcome back:"<<endl;
    string name;
    cout<<"Please input your Username:"<<endl;
    cin>>name;
    
    string password;
    cout<<"Please input your Password:"<<endl;
    cin>>password;
    string send_info="l|"+name+'|'+password+'|';
    int sending_re = (int)::send(client_socket_no,send_info.c_str(),(int)send_info.size(),0);
    cout<<"Client message length:"<<sending_re<<endl;
    
    if(sending_re > 0)
        fprintf(stderr, "\tSending message success, bytes: %d\n",sending_re);
    else{
        fprintf(stderr, "Sending message error\n");
    }
    
    cout<<"Wating for the server response......\n"<<endl;
    int socket_length = 0;
    char receive_buff[256];
    memset(receive_buff, 0, sizeof(receive_buff));
    socket_length = (int)::recv(client_socket_no, receive_buff, 256, 0);
    if(socket_length<0){
        perror("login_listen:\n");
    }
    string receive_status=receive_buff;
    if(socket_length > 0 && receive_status.substr(0,socket_length)=="l0"){
        cout<<"Username or password worong:"<<endl;
        print_1st_menu();
        login_1st_menu();
    }else if(socket_length > 0 &&receive_status.substr(0,socket_length)=="l1"){
        
        cout<<"login successfully!"<<endl;
        //cout<<receive_buff<<endl;
        show_online_friends(name);
        current_user.name=name;
        current_user.password=password;
        print_2nd_menu();
        login_2nd_menu();
        
    }
    return 0;
}
/************Show online users Function************************/
int show_online_friends(string name){
    
    
    char receive_buff[256];
    memset(receive_buff, 0, sizeof(receive_buff));
    if(recv(client_socket_no, receive_buff, sizeof(receive_buff), 0)==0){
        perror("receive friendlist:");
    };
    cout<<"Receiving friend list:"<<receive_buff<<endl;
    string temp=receive_buff;
    vector<string>friend_list;
    friend_list=split(temp, ";");
    cout<<"There are "<<(int)friend_list.size()-1<<" users online:"<<endl;
    for(int i=1;i<(int)friend_list.size();i++){
        cout<<friend_list[i]<<endl;
    }
    
    return 0;
}
/*********Exit the system*****************/
void exit_socket()
{
    shutdown(cc_listen_socket,SHUT_RDWR);
    shutdown(client_socket_no,SHUT_RDWR);
    close(cc_listen_socket);
    close(client_socket_no);
    cout<<endl<<"Socket is closing....."<<endl;
    cout<<endl<<"Bye Bye!"<<endl;
    exit(0);
}
/************Print out 2nd-login menu************************/
void print_2nd_menu()
{
    cout<<"--------------menu----------------"<<endl;
    cout<<"\t m friend message: send your friend a message"<<endl;
    cout<<"\t i:inviate a friend"<<endl;
    cout<<"\t ia:accept a friend request"<<endl;
    cout<<"\t logout:logout"<<endl;
    cout<<"--------------menu----------------"<<endl;
}
/************Login 2nd-login menu************************/
void login_2nd_menu(){
    
    struct timeval time_out={5,0};
    int fd_max_num;
    char receive_buff[256];
    char input_buff[3][256];
    socklen_t socket_length;
    string cmd;
    //string new_ip = getIPAddress();
    //    if(init_listen_socket()){
    //        fprintf(stderr, "Init listen socket on client is failed!\n");
    //        return;
    //    }
    //
    while(1){
        FD_ZERO(&fds);
        FD_SET(0,&fds);
        //FD_SET(0,&fds);
        fd_max_num=0;
        FD_SET(client_socket_no,&fds);
        if(fd_max_num <client_socket_no){
            fd_max_num = client_socket_no;
        }
        FD_SET(cc_listen_socket,&fds);
        if(fd_max_num <cc_listen_socket){
            fd_max_num = cc_listen_socket;
        }
        switch(select(fd_max_num+1,&fds,NULL,NULL,&time_out)){
            case -1:
                fprintf(stderr, "select error!\n");
                exit(1);
                break;
            case 0:
                break;
            default:
                if(FD_ISSET(client_socket_no,&fds)){
                    memset(receive_buff, 0, sizeof(receive_buff));
                    socket_length = (int)::recv(client_socket_no, receive_buff, 256, 0);
                    if(socket_length > 0){
                        cout<<"Receive message from client: "<<receive_buff<<endl;
                        //                        string temp=receive_buff;
                        //                        vector<string>friend_list;
                        //                        friend_list=split(temp, ";");
                        //                        if(friend_list[0]=="null"){
                        //                            cout<<"There are "<<(int)friend_list.size()-1<<" users online:"<<endl;
                        //                            for(int i=1;i<(int)friend_list.size();i++){
                        //                                cout<<friend_list[i]<<endl;
                        //                            }
                        //                        }
                    }
                    else if(socket_length < 0 ){
                        fprintf(stderr, "Receive message error\n");
                        break;
                    }
                    else{
                        fprintf(stderr, "Server exit and message client terminate\n");
                        exit(1);
                    }
                }
                else if(FD_ISSET(0,&fds)){
                    //cout<<"Your choice"<<endl;
                    //                    memset(input_buff, 0, sizeof(input_buff));
                    scanf("%s",input_buff[0]);
                    if(strcmp(input_buff[0],"logout")==0){
                        cmd="logout";
                    }
                    else{
                        scanf("%s %s",input_buff[1],input_buff[2]);
                        string flag=input_buff[0];
                        string name=input_buff[1];
                        string message=input_buff[2];
                        cmd=flag+"|"+name+"|"+message;
                    }
                    //cout<<cmd<<endl;
                    command_2nd_handler(cmd);
                    
                }
                //                else if(FD_ISSET(cc_listen_socket,&fds)){
                //                    cout<<"Sending message"<<endl;
                //
                //                }
        }
    }
    
}

int command_2nd_handler(string command){
    //cout<<command<<endl;
    vector<string>command_list;
    command_list=split(command,"|");
    string option=command_list[0];
    cout<<command_list[0]<<endl;
    if(option=="m"){
        cout<<"Send friend message: "<<endl;
        message(command_list[1],command_list[2]);
    }else if(option=="i"){
        cout<<"Inviate a friend:"<<endl;
        invite(command_list[1],command_list[2]);
        
    }else if(option=="ia"){
        cout<<"Accept all friend requests"<<endl;
        accept(command_list[1],command_list[2]);
    }else if(option=="logout"){
        logout();
        //cout<<"Logout"<<endl;
    }
    
    return 0;
}
/************Sending invitations*********************/
int invite(string name, string message){
    string client_inviation;
    //    cout<<"Please input the username and  messenge to your friend you want to invite:"<<endl;
    //    cout<<"Expamle: user1>>Hello_user1"<<endl;
    client_inviation=name+">>"+message;
    string send_info="i|"+current_user.name+'|'+client_inviation;
    send(client_socket_no,send_info.c_str(),(int)send_info.size(),0);
    cout<<"Sending invitation!"<<endl;
    return 1;
}

/************Accept invitations*********************/
int accept(string name, string message){
    
    string sendline="ia|"+current_user.name+"|"+name+"|"+message;
    send(client_socket_no,sendline.c_str(),(int)sendline.size(),0);
    
    //show_online_friends(current_user.name);
    //printf("accept someone\n");
    return 1;
}
/************Logout users*********************/
int logout(){
    
    char buffer[256];
    cout<<"Waiting for your back!"<<endl;
    string send_info="logout|"+current_user.name;
    send(client_socket_no,send_info.c_str(),(int)send_info.size(),0);
    memset(buffer, 0, sizeof(buffer));
    //recv(client_socket_no, buffer, 256, 0);
    //cout<<buffer<<endl;
    //shutdown(cc_listen_socket,SHUT_RDWR);
    //shutdown(client_socket_no,SHUT_RDWR);
    //close(cc_listen_socket);
    close(client_socket_no);
    FD_ZERO(&fds);
    client_socket_no = init_socket();
    if( client_socket_no< 0){
        fprintf(stderr, "initSock failed!\n");
        return 1;
    }else{
        printf("\n***************************WELCOME!***************************\n");
        print_1st_menu();
        login_1st_menu();
    }
    
    
    return 0;
}

int init_listen_socket(){
    
    // create socket;
    cc_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(cc_listen_socket == -1){
        fprintf(stderr,"Create socket failed.\n");
        exit_socket();
        exit(0);
    }
    struct sockaddr_in client_address;
    // bind socket and address;
    memset(&(client_address), 0, sizeof(client_address));
    client_address.sin_family = AF_INET;
    string ip=getIPAddress();
    int new_port = 5200;
    client_address.sin_addr.s_addr = inet_addr(ip.c_str());
    client_address.sin_port =new_port;
    // check bind;
    int reuse = 1;
    cout<<"client ip:"<<ip<<endl;
    setsockopt(cc_listen_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    if(::bind(cc_listen_socket, (struct sockaddr*)&(client_address), sizeof(client_address) )== -1){
        fprintf(stderr,  "Bind cc socket failed.\n");
        exit(0);
    }
    // bind successed;
    // listen to clients;
    if(listen(cc_listen_socket, 50) == -1){
        fprintf(stderr, "Listen socket failed\n");
        exit(0);
        return -1;
    }
    return 0;
}

/*************Get IP Address************************/
string getIPAddress(){
    string ipAddress="Unable to get IP Address";
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;
    // retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);
    if (success == 0) {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL) {
            if(temp_addr->ifa_addr->sa_family == AF_INET) {
                // Check if interface is en0 which is the wifi connection on the iPhone
                if(strcmp(temp_addr->ifa_name, "en0")){
                    ipAddress=inet_ntoa(((struct sockaddr_in*)temp_addr->ifa_addr)->sin_addr);
                }
            }
            temp_addr = temp_addr->ifa_next;
        }
    }
    // Free memory
    freeifaddrs(interfaces);
    return ipAddress;
}

int message(string to_name,string information){
    string send_info="m|"+current_user.name+'|'+to_name+'|'+information+'|';
    if(send(client_socket_no,send_info.c_str(),(int)send_info.size(),0)>0){
        cout<<"Sending message!"<<endl;
        cout<<send_info<<endl;
    }
    return 0;
    
    
}

void sig_int_handler(int sig_num){
    exit_socket();
    exit(0);
}



