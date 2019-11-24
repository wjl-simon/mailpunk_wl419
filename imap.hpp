#ifndef IMAP_H
#define IMAP_H
#include "imaputils.hpp"
#include <libetpan/libetpan.h>
#include <string>
#include <functional>
#include <map>

namespace IMAP {

  
class Session;
  
class Message {

  Session* session; // ptr to the coresponding session
  mailimap** ptrIMAP; // ptr to (the coresponding session's imap)
  uint32_t const UID;
  std::string msgBody;
  std::string msgSubject;
  std::string msgFrom;
  std::function<void()> updateUIFunction; // to refresh UI after deletion
  
public:

  Message(Session* session, mailimap** ptrimap, uint32_t uid,
          std::function<void()> updateuifunction);
  /**
   * Get the body of the message. You may chose to either include the headers or not.
   */
  std::string getBody();
  /**
   * Get one of the descriptor fields (subject, from, ...)
   */
  std::string getField(std::string fieldname);
  /**
   * Remove this mail from its mailbox
   */
  void deleteFromMailbox();
};


class Session {
  
  mailimap* imap; // a pointer to which will later point to a newed mailmap object
  std::string mailbox; // name of the mailbox
  uint32_t numMsgs; // number of msgs in the mailbox
  std::function<void()> updateUIFunction; // functional obj to update the UI
  
  /**
   * Get the mailbox status i.e. num of msgs, uid of the next mailbox, ect
   */
  void getMailboxStatus();
  
  /**
   * Get the UID of an msg
   */
  uint32_t getOneMsgUID(mailimap_msg_att* msg_att);
    
public:

  Message** msgList;// a list of all emails in the inbox

  Session(std::function<void()> updateUI);

  /**
   * Get all messages in the INBOX mailbox terminated by a nullptr (like we did in class)
   */
  Message** getMessages();

  /**
   * connect to the specified server (143 is the standard unencrypte imap port)
   */
  void connect(std::string const& server, size_t port = 143);

  /**
   * log in to the server (connect first, then log in)
   */
  void login(std::string const& userid, std::string const& password);

  /**
   * select a mailbox (only one can be selected at any given time)
   * 
   * this can only be performed after login
   */
  void selectMailbox(std::string const& mailbox);

  ~Session();
};

  
}

#endif /* IMAP_H */
