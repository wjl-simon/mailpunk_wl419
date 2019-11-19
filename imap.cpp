#include "imap.hpp"
#include "imaputils.hpp"
#include <libetpan/libetpan.h>
#include <string>
#include <functional>
#include <iostream>
#include <cstring>

using namespace IMAP;
using namespace std;

/*==========Message Class==========*/

/**
 * Constructor
 */
Message::Message(mailimap** ptrimap, uint32_t uid):ptrIMAP(ptrimap),UID(uid){}

/**
 * Get the body of the message. You may chose to either include the headers or not.
 */
std::string Message::getBody()
{
  /*
  mailimap_set* setWithThisMsgOnly = mailimap_set_new_single(UID); // a set with this msg only
  mailimap_fetch_type* msgFetchType
    = mailimap_fetch_type_new_fetch_att_list_empty(); // fetch type
  mailimap_section* section = mailimap_section_new(nullptr);
  mailimap_fetch_att* requestBody
    = mailimap_fetch_att_new_boody_peek_section(section); // body request
  mailimap_fetch_type_new_fetch_att_list_add(msgFetchType,requestBody);// Fill msgFetchType

  clist* result; // fetched result
    
  //--- 1. Fetching the body (msg content)
  int err = mailimap_uid_fetch(*ptrIMAP,setWithThisMsgOnly,&ressult);
  check_error(err,"could not fetch the body");

  //--- 2. Extract the body (msg content) from the fetched result
  mailimap_msg_att* msgAtt = (mailimap_msg_att*)clist_content(result);
  mailimap_msg_att_item* item = clist_content(msgAtt->att_list);
  
  if(item->att_type != MAILIMAP_MSG_ATT_ITEM_STATIC) return;

  if(item->att_data.att_static->att_type != MAILIMAP_MSG_ATT_BODY_SECTION) return;

  string msgContentTemp
    = item->att_data.att_static->att_data.att_body_section->sec_body_part; // msg content

  //--- 3. Update the msgBody
  msgBody.resize(msgContentTemp.length()), msgBody = msgContentTemp;
    
  //--- 4. Free the garbage
  mailimap_fetch_list_free(result); // the fetched result
  mailimap_fetch_att_free(requetBody); // the request
  mailimap_section_free(section);
  mailimap_set_free(setWithThisMsgOnly); // the msg set
  mailimap_fetch_type_free(msgFetchType); // the list of the requests

  return msgBody;
  */
}


/**
 * Get one of the descriptor fields (subject, from, ...)
 */
std::string Message::getField(std::string fieldname)
{
  /*
  mailimap_set* setWithThisMsgOnly = mailimap_set_new_single(UID); // a set with this msg only
  mailimap_fetch_type* msgFetchType
    = mailimap_fetch_type_new_fetch_att_list_empty(); // fetch type
  //mailimap_section* section = mailimap_section_new_header_fields_not(headerList);
  mailimap_fetch_att* requestHeader = mailimap_fetch_att_new_envelope();
  mailimap_fetch_type_new_fetch_att_list_add(msgFetchType,requestBody); // Fill msgFetchType

  clist* result;

  //--- 1. Fetching the envelope/header
  int err = mailimap_uid_fetch(*ptrIMAP,setWithThisMsgOnly,&ressult);
  check_error(err,"could not fetch the header");
  
  mailimap_msg_att* msgAtt = (mailimap_msg_att*)clist_content(result);
  mailimap_msg_att_item* item = clist_content(msgAtt->att_list);

  if(item->att_type != MAILIMAP_MSG_ATT_ITEM_STATIC) return;

  if(item->att_data.att_static->att_type != MAILIMAP_MSG_ATT_ENVELOPE) return;

  mailimap_envelope* msgEnv = item->att_data.att_static->att_env; // ptr to the envelope

  //--- 2. Free garbage
  mailimap_fetch_att_free(requestHeader);
  mailimap_fetch_type_free(msgFetchType);
  mailimap_set_free(setWithThisMsgOnly);

  //--- 3. Extract the strings
  switch(fieldname)
  {
  case "Date":
    msgDate = msgEnv->env_date;
    mailimap_fetch_list_free(result);
    break;
  case "Subject":
    msgSubject = msgEnv->env_subject;
    mailimap_fetch_list_free(result);
    break;
  case "Sender":
    clistiter* temp = msgEnv->envSender->snd_list;
    string name = clist_content(temp)->ad_personal_name;
    string addr = clist_content(temp)->ad_mailbox_name;
    string host = clist_content(temp)->ad_host_name;
    msgSender = "\"" + name + "\" <" + addr + "@" + host +">";
    mailimap_fetch_list_free(result);
    return msgSender;
    break;
  case "From":
    clistiter* temp = clist_begin(msgEnv->env_from->frm_list);
    string name = clist_content(temp)->ad_personal_name;
    string addr = clist_content(temp)->ad_mailbox_name;
    string host = clist_content(temp)->ad_host_name;
    msgFrom = "\"" + name + "\" <" + addr + "@" + host +">";
    mailimap_fetch_list_free(result);
    return msgFrom;
    break;
  case "Reply-To":
    clistiter* temp = clist_begin(msgEnv->env_repley_to->rt_list);
    string name = clist_content(temp)->ad_personal_name;
    string addr = clist_content(temp)->ad_mailbox_name;
    string host = clist_content(temp)->ad_host_name;
    msgReplyTo = "\"" + name + "\" <" + addr + "@" + host +">";
    mailimap_fetch_list_free(result);
    return msgReplyTo;
    break;
  case "To":
    clistiter* temp = clist_begin(msgEnv->env_to->to_list);    
    string name = clist_content(temp)->ad_personal_name;
    string addr = clist_content(temp)->ad_mailbox_name;
    string host = clist_content(temp)->ad_host_name;
    msgTo = "\"" + name + "\" <" + addr + "@" + host +">";
    mailimap_fetch_list_free(result);
    return msgTo;
    break;
  case "Cc":
    clistiter* temp = clist_begin(msgEnv->env_cc->cc_list);    
    string name = clist_content(temp)->ad_personal_name;
    string addr = clist_content(temp)->ad_mailbox_name;
    string host = clist_content(temp)->ad_host_name;
    msgCc = "\"" + name + "\" <" + addr + "@" + host +">";
    mailimap_fetch_list_free(result);
    return msgCc;
    break;
  case "Bcc":
    clistiter* temp = clist_begin(msgEnv->env_bcc->bcc_list);    
    string name = clist_content(temp)->ad_personal_name;
    string addr = clist_content(temp)->ad_mailbox_name;
    string host = clist_content(temp)->ad_host_name;
    msgBcc = "\"" + name + "\" <" + addr + "@" + host +">";
    mailimap_fetch_list_free(result);
    return msgBcc;
    break;
  case "In-Repley-To":
    msgInReplyTo = msgEnv->env_in_reply_to;
    return msgInReplyTo;
    mailimap_fetch_list_free(result);
    break;
  case "Message-ID":
    msgMessageID = msgEnv->env_message_id;
    return msgMessageID;
    mailimap_fetch_list_free(result);
    break;
  default:
    mailimap_fetch_list_free(result);
    return "N/A";
  }
  */
}


/**
 * Remove this mail from its mailbox
 */
void Message::deleteFromMailbox()
{
  /*
  //--- 1. Create a \Deleted flag instance
  mailimap_flag* flagDELETED = mailimap_flag_new_deleted();
  
  //--- 2. Add the \Deleted flag to a flag list (still not affect the object)
  mailimap_flag_list* flags = mailimap_flag_list_new_empty(); // Flags of this email
  int err = mailimap_flag_list_add(flags, flagDELETED);
  check_error(err, "could not add flags");

  //--- 3. Create a STORE description
  mailimap_store_att_flags* storeDesc  = mailimap_store_att_flags_new(1,1,flags);
  
  //--- 4. Alter the flags now!
  mailimap_set* setWithThisMsgOnly = mailimap_set_new_single(UID); // a set with this msg only
  err = mailimap_uid_store(*ptrIMAP,setWithThisMsgOnly,storeDesc);
  check_error(err,"could not alter flags");
  
  //--- 5. Remove this email from the mailbox
  err = mailimap_expunge(*ptrIMAP);
  check_error(err,"could not delete this email");

  //--- 6. Free garbage
  mailimap_store_att_flags_free(storeDesc); mailimap_set_free(setWithThisMsgOnly);
  mailimap_flag_free(flagDELETED); mailimap_flag_list_free(flags); 
  */
}



/*============Session Class============*/

/**
 * Get the mailbox status i.e. num of msgs, uid of the next mailbox, ect
 */
void Session::getMailboxStatus()
{
  //--- 1. List of all types of MAILBOX STATUS we want:
  // Create and fill the list of those STATUS types
  mailimap_status_att_list* mbStaTypes = mailimap_status_att_list_new_empty();
  // # of msgs
  int err = mailimap_status_att_list_add(mbStaTypes,MAILIMAP_STATUS_ATT_MESSAGES);
  check_error(err, "could not know number of emails");
  // # of recent msgs
  err = mailimap_status_att_list_add(mbStaTypes,MAILIMAP_STATUS_ATT_RECENT);
  check_error(err, "could not know number of recent emails");
  // UID of next mailbox
  err = mailimap_status_att_list_add(mbStaTypes,MAILIMAP_STATUS_ATT_UIDNEXT);
  check_error(err, "could not know UID of the next mailbox");
  // UID validit value of the mailbox
  err = mailimap_status_att_list_add(mbStaTypes,MAILIMAP_STATUS_ATT_UIDVALIDITY);
  check_error(err, "could not know UID of the next mailbox");
  // number of unseen msgs
  err = mailimap_status_att_list_add(mbStaTypes,MAILIMAP_STATUS_ATT_UNSEEN);
  check_error(err, "could not know number of unseen emails");
  
  //--- 2. Fetch those wanted STATUS of the mailbox from the server
  char str_temp[mailbox.length()+1]; strcpy(str_temp,mailbox.c_str());
  // mbStatus has all the specific STATUS info of all STATUS types about the mailbox
  mailimap_mailbox_data_status* mbStatus = mailimap_mailbox_data_status_new(str_temp,NULL);
  
  err = mailimap_status(imap, mailbox.c_str(), mbStaTypes, &mbStatus);
  check_error(err, "could not fetch the mailbox status");

  //--- 3. Etract the STATUS
  clistiter* cur; // iterator
  
  clist* ptrInfo = mbStatus->st_info_list;// STATUS are here
  int statusType; // the type of STATUS
  for(cur = clist_begin(ptrInfo); cur != nullptr; cur = clist_next(cur))
  {
    statusType = ((mailimap_status_info*)clist_content(cur))->st_att;
    switch(statusType)
    {
      case MAILIMAP_STATUS_ATT_MESSAGES:
        numMsgs = ((mailimap_status_info*)clist_content(cur))->st_value; break;
      case MAILIMAP_STATUS_ATT_RECENT:
        numRecentMsgs = ((mailimap_status_info*)clist_content(cur))->st_value; break;
      case MAILIMAP_STATUS_ATT_UIDNEXT:
        nextMailboxUID = ((mailimap_status_info*)clist_content(cur))->st_value; break;
      case MAILIMAP_STATUS_ATT_UIDVALIDITY:
        uidValid = ((mailimap_status_info*)clist_content(cur))->st_value; break;
      case MAILIMAP_STATUS_ATT_UNSEEN:
        numUnseenMsgs = ((mailimap_status_info*)clist_content(cur))->st_value; break;
    }
  }

  //--- 4. Free garbage
  mailimap_mailbox_data_status_free(mbStatus); // mailbox status
  mailimap_status_att_list_free(mbStaTypes);
}


/**
 * Get the UID of an msg
 */
uint32_t Session::getOneMsgUID(mailimap_msg_att* msg_att)
{
  mailimap_msg_att_item* item; // an item of the msg_att list
  
  for(clistiter* cur = clist_begin(msg_att->att_list); !cur; cur = clist_next(cur))
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
  :mailbox("mailbox1"),numMsgs(0),numRecentMsgs(0),nextMailboxUID(0),uidValid(0),numUnseenMsgs(0),
   msgList(nullptr)
{
  // Create a new IMAP session
  imap = mailimap_new(0,NULL);
  // List of UIDs of the already fetched emails
  fetchedMsgUID = clist_new();
}


/**
 *  Get all messages in the INBOX mailbox terminated by a nullptr (like we did in class)
 */
Message** Session::getMessages()
{
  //--- 0. Free the old email list
  if(msgList != nullptr && numMsgs > 0)
  {
    for(int i = 0; i < numMsgs; i++)
    {
      delete msgList[i];
      msgList[i] = nullptr;
    }
  }
  
  //--- 1. Get the mailbox STATUS
  getMailboxStatus();

  //--- 2. Fetching the UIDs of the unfetched message
  mailimap_set* msgSet;
  mailimap_fetch_type* TypeToFetch;
  mailimap_fetch_att* fetchAtt;
  clist* result; // the fetched result

  msgSet = mailimap_set_new_interval(1,0);
  TypeToFetch = mailimap_fetch_type_new_fetch_att_list_empty();
  fetchAtt = mailimap_fetch_att_new_uid(); // the attribute to fetch: UID
  mailimap_fetch_type_new_fetch_att_list_add(TypeToFetch,fetchAtt);

  int err = mailimap_fetch(imap,msgSet,TypeToFetch,&result); // fetching the UID
  check_error(err,"could not fetch emails");

  //--- 3. Extract the UIDs of the unfetched emails from the result and warp up into a Message obj
  mailimap_msg_att* msgAtt;
  uint32_t uid; uint32_t* ptruid; // the value of the current uid and the ptr to uid
  Message* msg; // a newed msg with UID=uid
  clistiter* cur1; clistiter* cur2; // iterators
  bool isFetchedBefore = false; // a flag if the msg being fetched is duplicated
  
  for(cur1 = clist_begin(result); !cur1; cur1 = clist_next(cur1))
  {
    msgAtt = (mailimap_msg_att*)clist_content(cur1);
    uid = getOneMsgUID(msgAtt); // get the uid
    
    if(uid == 0) continue; // check if the uid is 0
    
    isFetchedBefore = false; // check if the msg being fetched is already fetched before 
    for(cur2 = clist_begin(fetchedMsgUID); !cur2; cur2 = clist_next(cur2))
    {
      if(*(uint32_t*)clist_content(cur2) == uid)
      {
        isFetchedBefore = true;
        break;
      }
    }
    
    if(isFetchedBefore) continue;
    else // Update the fetchedMsgUID list
    {
      ptruid = new uint32_t, *ptruid = uid;
      
      err = clist_append(fetchedMsgUID,(void*)ptruid); // insert the uid into the fetchedMsgUID list
      string str2 = "could not record email with UID " + std::to_string(uid); 
      check_error(err,str2.c_str());
    }
  }
    
  //--- 4. Wrap up an array of all msgs in the inbox
  Message* msglist[numMsgs]; int i = 0;
  for(cur1 = clist_begin(fetchedMsgUID); !cur1; cur1 = clist_next(cur1))
  {
    msglist[i] = new Message(&imap, *(uint32_t*)clist_content(cur1));
    i++;
  }
  msgList = msglist;

  //--- 5. Free garbage
  mailimap_fetch_list_free(result); // fetched result
  mailimap_fetch_att_free(fetchAtt); mailimap_fetch_type_free(TypeToFetch);
  mailimap_set_free(msgSet);
  
  return msglist;
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
  this->mailbox.resize(mailbox.size()); this->mailbox = mailbox;
}
  
Session::~Session()
{
  // Free the message list
  for(int i = 0; i < numMsgs; i++) delete msgList[i];
  // Free the fetchedMsgUID
  clist_free(fetchedMsgUID);
  // Logout then free the imap session
  mailimap_logout(imap); mailimap_free(imap);
}


