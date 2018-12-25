/*Author:Bowen Li
 Course: CPDP
 Program: Server.cpp
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
int server_socket_no;
struct sockaddr_in client_addr;

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
class Invitation{
public:
    string from_name;
    string to_name;
    string message;
};
vector<User>User_list_server;
vector<Invitation>Invitation_list_server;
/**********FUNCTION DELCARATION***********************/
vector<User> read_user_info();//reading user_info_file function
vector<string> split(const string& str, const string& delim);//split function
string read_config_info();// read port no from config_file
string getIPAddress();// get ip address
void write_client_config(string ip, string port); //write back client config file
int init_socket();//init socket
void *thread_handler(void *socket);
int command_judge(string command,int client_socket_no);
int register_handler(string name,string password,int client_socket_no);
int find_user(vector<User> user_list,string str);
void write_user_info(string user_info);
int login_handler(string name,string password,int client_socket_no);
string login_user(vector<User> user_list,string str);
string show_online_user(string name);
int check_online_friends(string name, vector<string>friend_list);
int invite_handler(string from,string to_message);
int search_user_socket(string name);
int check_user_online(string name);
int accept_handler(string to_name,string from_name,string message);
Invitation check_invitation(string from_name,string to_name);
int add_friend(string name1, string name2);
void write_back_user_info(vector<User> user_list);
int logout_handler(string name,int client_socket_no);
int send_logout(string name);
int remove_online_user(string name);
int send_login(string name);
int message_handler(string from,string message, string to);
void sig_int_handler(int sig_num);
/**********MAIN  FUNCTION***********************/
int main(int argc, char *argv[]){
    server_socket_no=init_socket();
    cout<<server_socket_no<<endl;
    int sock;
    pthread_t pthread_id[50];
    int thread_num = 0;
    int ret;
    signal(SIGINT,sig_int_handler);
    while(1){
        socklen_t len = sizeof(struct sockaddr_in);
        //signal(SIGPIPE, SIG_IGN);
        sock = ::accept(server_socket_no, (struct sockaddr*)&client_addr, &len);
        if(sock < 0)
        {
            perror("accept:");
            continue;
        }
        cout<<"get a client, ip:"<<inet_ntoa(client_addr.sin_addr)<<","<<"port:"<<ntohs(client_addr.sin_port)<<endl;
        ret = pthread_create(&pthread_id[thread_num], NULL, thread_handler, &sock);
        //
       if (ret != 0)
        {
            fprintf(stderr, "create thread failed!\n");
            close(sock);
        }else{
            fprintf(stderr, "create thread[%d] success!\n", thread_num);
        }
        
        thread_num++;
    }
    return 0;
}


/**********Reading user_inforamtion from user_info_file*****************/
vector<User> read_user_info(){
    string str;
    vector<User>user_list;
    vector<string>user_result;
    ifstream user_info("./server_file/user_info_file");
    if(!user_info) {
        cout << "Cannot open input file.\n";
        exit(1);
    }
    
    while(user_info) {
        User user_regisrer;
        getline(user_info,str);
        if(str!=""&&str!="\n"){
            user_result=split(str, "|;");
            for(int i=0; i<user_result.size();i++){
                
                user_regisrer.name=user_result[0];
                user_regisrer.password=user_result[1];
                if(i>1)
                    user_regisrer.friend_list.push_back(user_result[i]);
            }
            
            user_list.push_back(user_regisrer);
        }
    }
    user_info.close();
    
    return user_list;
}

/**********Split function*****************/
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
/**********Reading config inforamtion from config_info_file*****************/
string read_config_info(){
    string port_no;
    string str;
    vector<string>config_result;
    ifstream config_info("./server_file/configration_file");
    if(!config_info) {
        cout << "Cannot open confige file.\n";
        exit(1);
    }
    
    while(config_info) {
        getline(config_info,str);
        config_result=split(str, ":.");
        port_no=config_result[1];
    }
    
    return port_no;
}
/**********get ip address*****************/
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
/**********wirte back server information to client*****************/
void write_client_config(string ip, string port){
    ofstream outfile("./client_file/configration_file");
    if(!outfile) {
        cout << "Cannot create confige file.\n";
        exit(1);
    }
    string config_info;
    config_info="servhost:"+ip+"\nservport:"+port+"\n";
    outfile<<config_info<<endl;
    outfile.close();
}
/**********Initailize the socket from server to client*****************/
int init_socket(){
    //create the socket
    string port_no;
    int socket_server;
    struct sockaddr_in addr;
    string ip_temp=getIPAddress();
    //cout<<ip_temp<<endl;
    if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror(": Can't get socket");
        exit(1);
    }
    port_no=read_config_info();
    // bind socket and address
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = stoi(port_no);
    addr.sin_addr.s_addr = inet_addr(ip_temp.c_str());
    if (::bind(socket_server,(struct sockaddr *)&addr,sizeof(addr))< 0) {
        perror("bind");
        exit(1);
    }
    cout<<"Server IP:"<<inet_ntoa(addr.sin_addr)<<endl;
    cout<<"Server Port:"<<addr.sin_port<<endl;
    //listen for client
    if (listen(socket_server, 5) < 0) {
        perror("listen");
        exit(1);
    }
    
    printf("Init_socket finished!\n");
    write_client_config(ip_temp,port_no);
    return socket_server;
}
/********** Threads handler t*****************/
void *thread_handler(void *socket){
    int client_socket_no = *((int*)socket);
    int socket_length;
    char  receive_buff[256];
    
    while(1){
        //::bzero(receive_buff, 256);
        memset(receive_buff, 0, sizeof(receive_buff));
        socket_length=(int)::recv(client_socket_no, receive_buff, 256, 0);
        
        if(socket_length > 0){
            //cout<<"Receive socket from client server:"<<client_socket_no<<endl;
            cout<<"Receive message from client:"<<receive_buff<<endl;
            string command=receive_buff;
            command_judge(command,client_socket_no);
            //reg_log_handler(receive_buff, client_socket);
        }
        //        else if(socket_length==0){
        //            return NULL;
        //        }
        else
        {
            cout<<"socket error"<<socket_length<<endl;
            return NULL;
        }
        cout<<"273["<<client_socket_no<<"]"<<endl;
    }
    
    
    return NULL;
}
/**********Initailize the socket from server to client*****************/
int command_judge(string command,int client_socket_no){
    string receive_command=command;
    vector<string>command_list;
    User user_temp;
    command_list=split(receive_command, "|");
    cout<<"The command flag: "<<command_list[0]<<endl;
    if(command_list[0]=="r"){
        cout<<"Receive regist command"<<endl;
        register_handler(command_list[1],command_list[2],client_socket_no);
    }
    else if(command_list[0]=="l"){
        login_handler(command_list[1],command_list[2],client_socket_no);
    }
    else if(command_list[0]=="i"){
        invite_handler(command_list[1], command_list[2]);
    }
    else if(command_list[0]=="ia"){
        accept_handler(command_list[1],command_list[2],command_list[3]);
    }
    else if(command_list[0]=="m"){
        message_handler(command_list[1], command_list[3], command_list[2]);;
    }
    else if(command_list[0]=="logout"){
        logout_handler(command_list[1],client_socket_no);;
    }
    else{
        cout<<"306"<<endl;
    }
    return 0;
}
/**********Regist handler*****************/
int register_handler(string name,string password,int client_socket_no){
    string user_infor;
    vector<User>user_list;
    int flag;
    user_list=read_user_info();
    flag=find_user(user_list, name);
    if(flag==1){
        cout<<"Name had been registed"<<endl;
        cout<<"Sending status:"<<500<<endl;
        if( ::send(client_socket_no,"500\0",strlen("500\0"),0)>0)
            fprintf(stderr, "Sending message success\n");
        else{
            fprintf(stderr, "Sending message error\n");
        }
    }
    else{
        cout<<"Regist successfully!!"<<endl;
        user_infor=name+"|"+password+"|";
        cout<<user_infor<<endl;
        cout<<"Sending status:"<<"200"<<endl;
        
        write_user_info(user_infor);
        if( ::send(client_socket_no,"200\0",strlen("200\0"),0)>0)
            fprintf(stderr, "Sending message success\n");
        else{
            fprintf(stderr, "Sending message error\n");
        }
    }
    
    
    return 0;
}
/****find users: find out return 1*****/
int find_user(vector<User> user_list,string str){
    int flag =0;
    for(int i=0; i<user_list.size();i++){
        if(user_list[i].name==str){
            flag=1;
        }
    }
    
    return flag;
}

void write_user_info(string user_info){
    ofstream outfile("./server_file/user_info_file",ios_base::app);
    outfile<<user_info<<endl;
    outfile.close();
}
/**********Login handler*****************/
int login_handler(string name,string password,int client_socket_no){
    User user_temp;
    string user_infor;
    vector<User>user_list;
    string login_pw;
    user_list=read_user_info();
    login_pw=login_user(user_list, name);
    if(login_pw==password){
        cout<<"Login successfully!"<<endl;
        cout<<"Sending status:"<<"l1"<<endl;
        user_temp.name=name;
        user_temp.password=password;
        user_temp.socket=client_socket_no;
        User_list_server.push_back(user_temp);
        user_temp.ip=inet_ntoa(client_addr.sin_addr);
        user_temp.port=to_string( ntohs(client_addr.sin_port));
        cout<<user_temp.ip<<endl<<user_temp.port<<endl;
        send(client_socket_no,"l1",strlen("l1"),0);
        string online_user;
        online_user=show_online_user(name);
        send(client_socket_no,online_user.c_str(),online_user.size(),0);
        //send_login(name);
    }
    else{
        cout<<"Connot login!"<<endl;
        cout<<"Sending status:"<<"l0"<<endl;
        ::send(client_socket_no,"l0",strlen("l0"),0);
    }
    
    return 0;
}
/*********Find out the user's password*****************/
string login_user(vector<User> user_list,string str){
    string password="";
    for(int i=0; i<user_list.size();i++){
        if(user_list[i].name==str)
            password=user_list[i].password;
        
    }
    
    return password;
}
/*********Find out the onlie users*****************/
string show_online_user(string name){
    string online_user="null;";
    vector<User> user_list=read_user_info();
    vector<string> friend_list;
    for(int i=0;i<user_list.size();i++){
        if(user_list[i].name==name)
            friend_list=user_list[i].friend_list;
    }
    
    for(int i=0;i<User_list_server.size();i++){
        cout<<"Name in server list"<<User_list_server[i].name<<endl;
        if(check_online_friends(User_list_server[i].name,friend_list)){
            online_user=online_user+User_list_server[i].name+";";
        }
    }
    
    
    //cout<<"Online users:"<<online_user<<endl;
    return online_user;
}
/*********Find out the onlie friends*****************/
int check_online_friends(string name, vector<string>friend_list){
    int flag=0;
    for(int i=0;i<friend_list.size();i++){
        if(friend_list[i]==name)
            flag=1;
    }
    
    return flag;
    
}
/*********Invite Friends*****************/
int invite_handler(string from,string to_message){
    
    cout<<from<<endl<<to_message<<endl;
    string from_feedback;
    string to_feedback;
    vector<string>tomessage=split(to_message,">>");
    int client_socket_from = search_user_socket(from);
    int client_socket_to = search_user_socket(tomessage[0]);
    cout<<"This invitation send from "<<client_socket_from<<" to "<<client_socket_to<<endl;
    Invitation inva_temp;
    inva_temp.from_name=from;
    inva_temp.to_name=tomessage[0];
    inva_temp.message=tomessage[1];
    cout<<"Inviate message from "<<inva_temp.from_name<<" to "<<inva_temp.to_name<<" and say that "<<inva_temp.message<<endl;
    //check if user_to is online
    if(check_user_online(inva_temp.to_name) == 1){
        from_feedback= "Find the online user: "+inva_temp.to_name+" and the invitation had been sent!";
        cout<<from_feedback<<endl;
        Invitation_list_server.push_back(inva_temp);
        //send back to the from server
        ::send(client_socket_from,from_feedback.c_str(),(int)from_feedback.size(),0);
        to_feedback=inva_temp.from_name+">>"+inva_temp.message;
        cout<<to_feedback<<endl;
        //send back to the to server
        ::send(client_socket_to,to_feedback.c_str(),(int)to_feedback.size(),0);
        
    }else{
        from_feedback="The users "+inva_temp.to_name+" is not online";
        cout<<from_feedback<<endl;
        ::send(client_socket_from,from_feedback.c_str(),(int)from_feedback.size(),0);
    }
    cout<<"Invitation is done"<<endl;
    cout<<"Form "<<from<<" to "<<inva_temp.to_name<<endl;
    return 1;
}
/************Search online users*********************/
int search_user_socket(string name){
    
    vector<User>::iterator it;
    cout<<"Searching user's socket:"<<name<<endl;
    
    for (it=User_list_server.begin(); it !=User_list_server.end(); ++it)
    {
        if (it->name==name)
        {
            cout<<"Find the socket:"<<it->socket<<endl;
            return it->socket;
        }
    }
    
    return 0;
}
/************Check whether users is online*********************/
int check_user_online(string name){
    
    vector<User>::iterator it;
    for (it = User_list_server.begin(); it !=User_list_server.end(); ++it)
    {
        if (it->name==name)
        {
            cout<<"Find this user is online"<<endl;
            return 1;
        }
    }
    return -1;
}
/************Accept friends request handler*********************/
int accept_handler(string to_name,string from_name,string message){
    Invitation invi;
    cout<<"From name:"<<from_name<<" To name: "<<to_name<<endl;
    cout<<"The user:"<<to_name<<" is processing invitation "<<"from "<<from_name<<endl;
    invi=check_invitation(from_name,to_name);
    int socket_to=search_user_socket(to_name);
    int socket_from=search_user_socket(from_name);
    string to_feedback,from_feedback;
    cout<<"From name:"<<from_name<<" To name: "<<to_name<<endl;
    from_feedback= "Your friends "+to_name+" had received you invitation";
    cout<<from_feedback<<endl;
    ::send(socket_from,from_feedback.c_str(),(int)from_feedback.size(),0);
    to_feedback="You accept your friend:"+from_name+" invitation";
    cout<<to_feedback<<endl;
    string online_user;
    //    online_user=show_online_user(to_name);
    //    send(socket_to,online_user.c_str(),online_user.size(),0);
    //send back to the to server
    send(socket_to,to_feedback.c_str(),(int)to_feedback.size(),0);
    add_friend(from_name, to_name);
    //    string online_user;
    //    online_user=show_online_user(from_name);
    //    ::send(socket_from,online_user.c_str(),online_user.size(),0);
    return 1;
}
/************Check one user's friend request*********************/
Invitation check_invitation(string from_name,string to_name){
    
    Invitation temp;
    //temp.message="No found";
    for (int i = 0; i<(int)Invitation_list_server.size();i++)
    {
        cout<<"The "<<i<<"th invatation "<<Invitation_list_server[i].from_name<<">>"<<Invitation_list_server[i].to_name<<endl;
        if (Invitation_list_server[i].to_name==to_name && Invitation_list_server[i].from_name==from_name)
        {
            cout<<"Find out the "<<i<<"th invatation:"<<endl;
            //Invitation_list_server.erase(Invitation_list_server.begin()+i);
            temp=Invitation_list_server[i];
        }
    }
    return temp;
}
/**************Add friends *************************/
int add_friend(string name1, string name2){
    
    cout<<"We are adding friends"<<endl;
    vector<User>user_list;
    user_list=read_user_info();
    for(int i=0;i<user_list.size();i++){
        if(user_list[i].name==name1)
            user_list[i].friend_list.push_back(name2);
        else if(user_list[i].name==name2)
            user_list[i].friend_list.push_back(name1);
    }
    write_back_user_info(user_list);
    
    return 0;
}
/**************Write back latest user information *************************/
void write_back_user_info(vector<User> user_list){
    string user_info;
    string final_user_info="";
    string friend_info;
    ofstream outfile("./server_file/user_info_file");
    if(!outfile) {
        cout << "Cannot open user_info file.\n";
        exit(1);
    }
    vector<User>::iterator it;
    for (it = user_list.begin(); it !=user_list.end(); ++it){
        user_info="";
        friend_info="";
        user_info=it->name+'|'+it->password+'|';
        for(int i=0;(int)i<it->friend_list.size();i++){
            friend_info=friend_info+it->friend_list[i]+';';
        }
        user_info=user_info+friend_info;
        final_user_info=final_user_info+user_info+"\n";
    }
    cout<<final_user_info<<endl;
    outfile<<final_user_info<<endl;
    outfile.close();
}
/**************Write back latest user information *************************/
int logout_handler(string name,int client_socket_no){
    
    send_logout(name);
    remove_online_user(name);
    cout<<name+" Bye!"<<endl;
    //string send_info=name+" had logged out!";
    //send(client_socket_no,send_info.c_str(),(int)send_info.size(),0);
    
    
    return 0;
}

/**************Send to friends logout information*************************/
int send_logout(string name){
    
    
    string information="The user "+name+" is offline.";
    cout<<information<<endl;
    cout<<"Current user number:"<<User_list_server.size()<<endl;
    for (int i=0; i<(int)User_list_server.size();i++)
    {
        int sockfd= search_user_socket(User_list_server[i].name);
        cout<<"The socket:"<<sockfd<<"  "<<User_list_server[i].socket<<endl;
        send(sockfd,information.c_str(),(int)information.length(),0);
    }
    
    return 1;
}
/**************Remove users form current user on server*************************/
int remove_online_user(string name){
    
    for (int i=0; i<(int)User_list_server.size(); i++)
    {
        if (User_list_server[i].name==name)
        {
            User_list_server.erase(User_list_server.begin()+i);
            cout<<name+" removed"<<endl;
            return 0;
        }
    }
    
    
    return 1;
}
/**************Send friends user is online*************************/
int send_login(string name){
    
    vector<User> user_list=read_user_info();
    vector<string> friend_list;
    string information="The user "+name+" is online.\n";
    cout<<information<<endl;
    cout<<"Current user number:"<<User_list_server.size()<<endl;
    for(int i=0;i<user_list.size();i++){
        if(user_list[i].name==name)
            friend_list=user_list[i].friend_list;
    }
    
    for(int i=0;i<User_list_server.size();i++){
        cout<<"online users:"<<User_list_server[i].name<<endl;
        if(check_online_friends(User_list_server[i].name,friend_list)){
            int sockfd= search_user_socket(name);
            cout<<"Find out the user's socket:"<<sockfd<<endl;
            send(sockfd,information.c_str(),(int)information.length(),0);
        }
    }
    
    return 1;
}

int message_handler(string from,string message, string to){
    
    cout<<"The user:"<<from<<" is sending message!"<<endl;
    int socket_to=search_user_socket(to);
    int socket_from=search_user_socket(from);
    if(check_user_online(to)==1){
        string to_feedback,from_feedback;
        from_feedback= "message sent\n";
        cout<<from_feedback<<endl;
        //send back to the from server
        ::send(socket_from,from_feedback.c_str(),(int)from_feedback.size(),0);
        to_feedback="You receive your friend- "+from+"'s message:\n"+message;
        cout<<to_feedback<<endl;
        //send back to the to server
        ::send(socket_to,to_feedback.c_str(),(int)to_feedback.size(),0);
    }
    else{
        
        string from_feedback;
        from_feedback="You friend:"+to+" is not online\n";
        cout<<from_feedback<<endl;
        //send back to the to server
        ::send(socket_from,from_feedback.c_str(),(int)from_feedback.size(),0);
    }
    
    
    return 0;
}
void sig_int_handler(int sig_num){
    
    shutdown(server_socket_no,SHUT_RDWR);
    close(server_socket_no);
    cout<<endl<<"waiting for your back!";
    exit(0);
}








