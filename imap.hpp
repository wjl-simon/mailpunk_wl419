#ifndef IMAP_H
#define IMAP_H
#include "imaputils.hpp"
#include <libetpan/libetpan.h>
#include <string>
#include <functional>
#include <map>

namespace IMAP {
class Message {

  mailimap** ptrIMAP; // ptr to the coresponding IMAP session
  uint32_t const UID;
  std::string msgBody;
  std::string msgSubject;
  std::string msgDate;
  std::string msgFrom;
  std::string msgSender;
  std::string msgTo;
  std::string msgReplyTo; // Reply-To field
  std::string msgCc;
  std::string msgBcc;
  std::string msgInReplyTo; // In-Reply-To field
  std::string msgMessageID; // Message-ID field

  
public:
    /**
     * Constructor
     */
    Message(mailimap** ptrimap, uint32_t uid);
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
  std::string mailbox;
  uint32_t numMsgs;
  uint32_t numRecentMsgs;
  uint32_t nextMailboxUID;
  uint32_t uidValid;
  uint32_t numUnseenMsgs;
  clist* fetchedMsgUID; // a list of UIDs of the fetched msgs that are already in the mailbox
  Message** msgList; // a list of all emails in the inbox
  
  /**
   * Get the mailbox status i.e. num of msgs, uid of the next mailbox, ect
   */
  void getMailboxStatus();
  
  /**
   * Get the UID of an msg
   */
  uint32_t getOneMsgUID(mailimap_msg_att* msg_att);
  
  
public:
	Session(std::function<void()> updateUI);

	/**
	 * Get all messages in the INBOX mailbox terminated by a nullptr (like we did in class)
	 */
	Message** getMessages();

	/**
	 * connect to the specified server (143 is the standard unencrypte imap port)
	 */
	void connect(std::string const& server = "mailpunk.lsds.uk", size_t port = 143);

	/**
	 * log in to the server (connect first, then log in)
	 */
	void login(std::string const& userid = "wl419mail", std::string const& password = "5e14d17");

	/**
	 * select a mailbox (only one can be selected at any given time)
	 * 
	 * this can only be performed after login
	 */
	void selectMailbox(std::string const& mailbox);

	~Session();
};


/**
* a map from an int key to MAILIMAP_STATUS_ATT_XXX
*/
//std::map<int, std::string> const StatusCode;


  
}

#endif /* IMAP_H */
