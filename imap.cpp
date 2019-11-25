#include "imap.hpp"
#include "imaputils.hpp"
#include <libetpan/libetpan.h>
#include <string>
#include <functional>
#include <iostream>

using namespace IMAP;
using namespace std;

/*==========Message Class==========*/

/**
 * Constructor
 */
Message::Message(Session* session, mailimap** ptrimap, uint32_t uid,
                 std::function<void()> updateuifunction)
  :session(session), ptrIMAP(ptrimap),UID(uid),updateUIFunction(updateuifunction),
   msgBody("<null>"),msgFrom("\"Anonymous\""),msgSubject("<No Subject>")
{
  //--- 0. Representing the data structure
  mailimap_set* setWithThisMsgOnly = mailimap_set_new_single(UID); // a set with this msg only
  mailimap_fetch_type* msgFetchType = mailimap_fetch_type_new_fetch_att_list_empty(); // fetch type
  mailimap_section* section = mailimap_section_new(NULL);
  
  mailimap_fetch_att* requestBody
    = mailimap_fetch_att_new_body_peek_section(section); // body request
  mailimap_fetch_att* requestHeader = mailimap_fetch_att_new_envelope(); // Header request
  
  int err =mailimap_fetch_type_new_fetch_att_list_add(msgFetchType,requestHeader);
  check_error(err,"could not fetch the header");
  err =mailimap_fetch_type_new_fetch_att_list_add(msgFetchType,requestBody);
  check_error(err,"could not fetch the body");
    
  //--- 1. Fetching the msg body + header
  clist* result; // fetched result
  err = mailimap_uid_fetch(*ptrIMAP,setWithThisMsgOnly, msgFetchType, &result);
  check_error(err,"could not fetch the body");

  //--- 2. Extract the body + header from the fetched result
  mailimap_msg_att* msgAtt = (mailimap_msg_att*)clist_content(clist_begin(result));
  mailimap_msg_att_item* item;
  mailimap_envelope* msgEnv;  
  // There's one msg indeed
  for(clistiter* cur = clist_begin(msgAtt->att_list); cur; cur = clist_next(cur))
  {
    item = (mailimap_msg_att_item*)clist_content(cur);
    if(item->att_type != MAILIMAP_MSG_ATT_ITEM_STATIC) continue;
    
    // Get Header: from and subject
    if(item->att_data.att_static->att_type == MAILIMAP_MSG_ATT_ENVELOPE)
    {
      msgEnv = item->att_data.att_static->att_data.att_env; // ptr to the envelope
      
      if(msgEnv->env_from->frm_list != NULL) // get From
      {
        string sender; // temporary sender name + email address
        mailimap_address* temp;
        
        clist* fromList = msgEnv->env_from->frm_list;
        for(clistiter* cur = clist_begin(fromList); cur; cur = clist_next(cur))
          {
            temp = (mailimap_address*)clist_content(cur);
            if(temp->ad_personal_name!=NULL) // extract email name
            {
              sender.append("\""); sender.append(temp->ad_personal_name); // sender name
              sender.append("\"");
            }
            else
              sender.append("\"Anonymous\"");
            
            if(temp->ad_mailbox_name!=NULL && temp->ad_host_name!=NULL) // extract email address
            {
              sender.append(temp->ad_mailbox_name); sender.append("@"); // sender addr
              sender.append(temp->ad_host_name); // sender host 
            }
            else
              sender.append("<Unknown>");
          }
        msgFrom = sender; // update the sender name to the member
      }
      
      if(msgEnv->env_subject != NULL) // get Subject
        msgSubject = msgEnv->env_subject;
    }
    // Get body
    if(item->att_data.att_static->att_type != MAILIMAP_MSG_ATT_BODY_SECTION) continue;

    if(item->att_data.att_static->att_data.att_body_section->sec_body_part != NULL)
      msgBody = item->att_data.att_static->att_data.att_body_section->sec_body_part;
    else
      msgBody = "N/A";
  }

    
  //--- 4. Free the garbage
  mailimap_fetch_list_free(result); // the fetched result
  // The following leads to segmentation fault so I must comment them out!
  //mailimap_fetch_att_free(requestBody); // the request for Body
  //mailimap_fetch_att_free(requestHeader); // the request for header
  //mailimap_section_free(section);
  mailimap_fetch_type_free(msgFetchType); // the list of the requests
  mailimap_set_free(setWithThisMsgOnly); // the msg set
}


/**
 * Get the body of the message. You may chose to either include the headers or not.
 */
std::string Message::getBody() { return msgBody; }


/**
 * Get one of the descriptor fields (subject, from, ...)
 */
std::string Message::getField(std::string fieldname)
{
  if(fieldname == "From")
    return msgFrom;
  else if(fieldname == "Subject")
    return msgSubject;
  else
    return "N/A";
}


/**
 * Remove this mail from its mailbox
 */
void Message::deleteFromMailbox()
{
  // Create a \Deleted flag instance
  mailimap_flag* flagDELETED = mailimap_flag_new_deleted();
  
  // Add the \Deleted flag to a flag list (still not affect the object)
  mailimap_flag_list* flags = mailimap_flag_list_new_empty(); // Flags of this email
  int err = mailimap_flag_list_add(flags, flagDELETED);
  check_error(err, "could not add flags");

  // Create a STORE description
  mailimap_store_att_flags* storeDesc  = mailimap_store_att_flags_new_set_flags(flags);
   
  // Alter the flags now!
  mailimap_set* setWithThisMsgOnly = mailimap_set_new_single(UID); // a set with this msg only
  err = mailimap_uid_store(*ptrIMAP,setWithThisMsgOnly,storeDesc);
  check_error(err,"could not alter flags");
  
  // Remove this email from the mailbox in the server
  err = mailimap_expunge(*ptrIMAP);
  check_error(err,"could not delete this email");

  // Free garbage
  mailimap_store_att_flags_free(storeDesc); mailimap_set_free(setWithThisMsgOnly);
  mailimap_flag_free(flagDELETED); mailimap_flag_list_free(flags); 

  // Drop the old msgList in the corresponding session since the updateUIFunction()
  // will do another Session::getMessage() again
  for(int i = 0; session->msgList[i]; i++)
  {
    if(session->msgList[i] == this) // don't delete yourself before deleting all others!
      continue;
    else
      delete session->msgList[i];
  }
  delete [](session->msgList); session->msgList = nullptr;

  // Update UI
  updateUIFunction();

  // Locally delete thie email that is being deleted in the server
  delete this;
}



/*============Session Class============*/

/**
 * Get the mailbox status i.e. num of msgs in the inbox
 */
void Session::getMailboxStatus()
{
  mailimap_status_att_list* mbStaTypes = mailimap_status_att_list_new_empty();
  int err = mailimap_status_att_list_add(mbStaTypes,MAILIMAP_STATUS_ATT_MESSAGES);// # of msgs
  check_error(err, "could not get the number of emails");
 
  mailimap_mailbox_data_status* mbStatus;
  // Using the following leads to memory leak!!!
  //char mbName_temp[mailbox.length()+1];
  //strcpy(mbName_temp,mailbox.c_str());
  //mailimap_mailbox_data_status* mbStatus= mailimap_mailbox_data_status_new(mbName_temp,NULL);

  // Fetchting msg numbers
  err = mailimap_status(imap, mailbox.c_str(), mbStaTypes, &mbStatus);
  check_error(err, "could not fetch the mailbox status");

  // Extract the msg number in the mailbox
  clist* ptrInfo = mbStatus->st_info_list;// STATUS is here
  int statusType; // the type of STATUS
  for(clistiter* cur = clist_begin(ptrInfo); cur; cur = clist_next(cur))
  {
    statusType = ((mailimap_status_info*)clist_content(cur))->st_att;
    
    if(statusType == MAILIMAP_STATUS_ATT_MESSAGES)
      numMsgs = ((mailimap_status_info*)clist_content(cur))->st_value;
  }

  // Free garbage
  mailimap_mailbox_data_status_free(mbStatus); // mailbox status
  mailimap_status_att_list_free(mbStaTypes);
}


/**
 * Get the UID of an msg
 */
uint32_t Session::getOneMsgUID(mailimap_msg_att* msg_att)
{
  mailimap_msg_att_item* item; // an item in the msg_att list
  
  for(clistiter* cur = clist_begin(msg_att->att_list); cur; cur = clist_next(cur))
  {
    item = (mailimap_msg_att_item*)clist_content(cur);
    
    if(item->att_type != MAILIMAP_MSG_ATT_ITEM_STATIC) continue;

    if(item->att_data.att_static->att_type != MAILIMAP_MSG_ATT_UID) continue;

    return item->att_data.att_static->att_data.att_uid;
  }
  
  return 0;
}


/**
 *  Constructor
 */
Session::Session(std::function<void()> updateUI)
  :mailbox("mailbox1"),numMsgs(0),msgList(nullptr),updateUIFunction(updateUI)
{
  // Create a new IMAP session
  imap = mailimap_new(0,NULL);
}


/**
 *  Get all messages in the INBOX mailbox terminated by a nullptr (like we did in class)
 */
Message** Session::getMessages()
{
  //--- 1. Get the mailbox STATUS
  getMailboxStatus(); // effectively getting the num of msgs in the inbox

  if(numMsgs == 0) return nullptr; // do not fetch mail list if there's no email at all
  
  //--- 2. Fetching the UIDs of the unfetched message
  mailimap_set* msgSet;
  mailimap_fetch_type* TypeToFetch;
  mailimap_fetch_att* fetchAtt;

  msgSet = mailimap_set_new_interval(1,0);
  TypeToFetch = mailimap_fetch_type_new_fetch_att_list_empty();
  fetchAtt = mailimap_fetch_att_new_uid(); // the attribute to fetch: UID
  int err = mailimap_fetch_type_new_fetch_att_list_add(TypeToFetch,fetchAtt);
  check_error(err,"could not fetch emails");

  clist* result; // the fetched result
  err = mailimap_fetch(imap,msgSet,TypeToFetch,&result); // fetching the UID
  check_error(err,"could not fetch emails");
  
  //--- 3. Extract the UIDs of the unfetched emails from the result and warp up into a Message obj
  mailimap_msg_att* msgAtt;
  uint32_t uid; // the value of the current uid and the ptr to uid
  
  Message** msglist = new Message* [numMsgs + 1]; // temporary array of all msgs in the inbox
  int actualNum = 0; // the actual number in the msglist, maybe <= the msgNum due to uid==0 
  for(clistiter* cur1 = clist_begin(result); cur1; cur1 = clist_next(cur1))
  {
    msgAtt = (mailimap_msg_att*)clist_content(cur1);
    uid = getOneMsgUID(msgAtt); // get the uid
    
    if(uid != 0) // only instantiate when uid is nonzero
    {
      msglist[actualNum] = new Message(this,&imap,uid,updateUIFunction);
      actualNum++;
    }
  }
  numMsgs = actualNum;
  msglist[actualNum] = nullptr; // append a nullptr after the last fetchable msg
  msgList = msglist; // update the data member
  
  //--- 5. Free garbage
  mailimap_fetch_list_free(result); // fetched result
  //mailimap_fetch_att_free(fetchAtt); // segmentation fault!
  mailimap_fetch_type_free(TypeToFetch);
  mailimap_set_free(msgSet);
  
  return msgList;
}


/**
 * connect to the specified server (143 is the standard unencrypte imap port)
 */
void Session::connect(std::string const& server, size_t port)
{
  // Connect to the server
  int err = mailimap_socket_connect(imap,server.c_str(), port);
  check_error(err, "could not connect to server");
}


/**
 * log in to the server (connect first, then log in)
 */
void Session::login(std::string const& userid, std::string const& password)
{
  // Login to the mailpunk
  int err = mailimap_login(imap, userid.c_str(), password.c_str());
  check_error(err, "could not login");
}


/**
 * select a mailbox (only one can be selected at any given time)
 * 
 * this can only be performed after login
 */
void Session::selectMailbox(std::string const& mailbox)
{
  // Select a mailbox
  int err = mailimap_select(imap, mailbox.c_str());
  check_error(err, "could not select this mailbox");
  // Preserve the mailbox name
  this->mailbox = mailbox;
}


Session::~Session()
{
  // Free the message list
  if(msgList != nullptr)
  {
    for(int i = 0; msgList[i]; i++) delete msgList[i];

    delete []msgList;
  }
  // Logout then free the imap session
  mailimap_logout(imap); mailimap_free(imap);
}
