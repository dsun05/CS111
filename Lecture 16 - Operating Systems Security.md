# Lecture 16: Operating System Security

## 1. Introduction to Operating System Security

### 1.1. The Role of the Operating System in Security
The **Operating System (OS)** is a critical component of computer security because it serves as the primary interface between the user and the computer's hardware.

*   **The OS as the lowest visible software layer:** The OS presents the user with the API that defines what the underlying computer is capable of doing.
*   **Proximity to and control over hardware:** The OS has the privileged ability to command the hardware to perform any action it is capable of. Since nothing on a computer happens without hardware involvement, control over the OS equates to control over the entire machine.
*   **The principle that an unprotected OS leads to an unprotected machine:** If an attacker can illegitimately influence the OS, they gain nearly complete control over the computer. A security bug in the OS is particularly severe because it typically cannot be compensated for by software running at higher levels.

***

### 1.2. Importance of OS Security
The OS is central to security because it manages and mediates access to all fundamental system resources. If the OS doesn't perform these tasks securely, almost anything can go wrong.

*   **OS control over critical resources:**
    *   **Application memory:** The OS can read or write any page of memory belonging to any process. While it is not supposed to, it has the ability to give one process access to another process's memory.
    *   **Processor scheduling:** The OS scheduler decides which processes run and when. A compromised OS could refuse to run a legitimate process for malicious reasons.
*   **The role of the OS in ensuring resource integrity:** Users rely on the OS to provide honest access to resources. When a user requests to open a file (e.g., `~/foo`), they trust the OS to provide the correct file and its actual contents. If the OS is dishonest, the user has no way of knowing if they were given a different file or altered content. This applies to all I/O, including network traffic and device recordings.
*   **The foundational assumption of a secure OS for higher-level security systems:** When building secure software, developers must work under the assumption that the underlying OS is secure. If the OS is compromised, it becomes nearly impossible to guarantee any security for applications running on top of it.

***

### 1.3. Key Security Definitions

#### 1.3.1. Security and Protection
*   **Security:** This refers to a **policy** that defines the desired behavior of a system—what should and should not be allowed to happen.
    *   *Example Policy:* "No unauthorized user may access this file."
*   **Protection:** This refers to the **mechanisms** used to implement and enforce a security policy.
    *   *Example Mechanism:* "The operating system checks the user's identity against the file's access permissions and only allows the access if they match."

***

#### 1.3.2. Vulnerabilities and Exploits
*   **Vulnerability:** A **weakness** in software, hardware, or a protocol that an attacker can use to violate a security policy. Software is filled with vulnerabilities, but in practice, most are never actively used by an attacker.
*   **Exploit:** An **actual incident** where an attacker takes advantage of a specific vulnerability on a real system to cause an undesirable effect. The term also refers to the piece of software or series of steps used to trigger the vulnerability.

***

#### 1.3.3. Trust
*   **The concept of granting privileges based on identity:** Trust is a fundamental security concept where you perform actions for certain parties (those you trust) that you would not perform for others.
*   **Challenges of implementing trust in a computer system:**
    *   **Expressing trust in a digital format (bits):** Trust must ultimately be represented as data (bits). Deciding what these bits mean and why they are set is a complex problem.
    *   **Verifying identity:** To make a trust-based decision, you must first be sure of the identity of the party you are dealing with, which is especially difficult in remote interactions.
    *   **Handling situational and dynamic trust:** Trust can be conditional. For example, an employee might be trusted to enter a building at 10 AM but not at 2 AM. Trust can also change over time (e.g., when an employee leaves the company), and the system must be updated to reflect this.

***

#### 1.3.4. The Operating System and Trust
*   **The inherent necessity of trusting the OS:** As a user or application, you have no choice but to trust the OS. When you make a system call, you assume the OS will perform the requested action honestly.
*   **The OS's complete control over hardware, processes, and I/O:** The OS can kill your processes, lie about the success of an operation, and control all hardware. If the OS is malicious, there is very little an application can do.
*   **The significance of an OS compromise:** Compromising an OS grants an attacker a tremendous amount of power over the machine and its users. If you do not trust your current OS (e.g., Windows, Linux, macOS), your only recourse is to switch to another one, but ultimately you must place your trust in some OS developer.

***

#### 1.3.5. Authentication and Authorization
*   **Authentication:** The process of determining and verifying an entity's identity. It answers the question, "Who is this?"
*   **Authorization:** The process of determining if an authenticated entity has the necessary permissions to perform a specific action on a resource. It answers the question, "Is this user allowed to do this?"
*   **The dependency of authorization on successful authentication:** As a rule, proper authorization cannot be performed without first completing a proper authentication. Bad authentication leads to bad authorization.

***

## 2. Authentication

### 2.1. Real-World Authentication Analogies
We perform authentication constantly in our daily lives using several methods that have cyber analogs.
*   **By recognition:** Identifying someone by recognizing their face or voice.
*   **By credentials:** Verifying identity using an official document, like a police officer checking a driver's license.
*   **By knowledge:** Granting access because someone knows a secret, such as the password to enter a clubhouse.
*   **By location:** Trusting someone is a DMV employee because they are standing behind the official counter at the DMV office.

***

### 2.2. Computer-Based Authentication
Computer authentication differs from human authentication due to the nature of computers.
*   **Comparison to human authentication:** Computers are not as flexible or "smart" as humans and can be fooled in ways a person would not be (e.g., facial recognition tricked by a photo). However, they are extremely fast and accurate at performing complex mathematical computations, which can be leveraged for strong authentication methods.
*   **Need to authenticate non-human entities:** Authentication is often required for non-human entities, such as a process, a server, or another machine, which do not have physical characteristics like a face.

***

### 2.3. Identities in Operating Systems
*   **The use of a User ID as the primary identifier:** In most operating systems, an identity is ultimately represented by a number, the **User ID**, which uniquely identifies a user on the system.
*   **Processes inheriting the User ID of their parent process:** When a process creates a new process (e.g., via `fork` in Linux), the new child process typically inherits the same User ID as its parent.
*   **The security model where all of a user's processes share the same privileges:** A significant drawback of this model is that every process running on behalf of a user has the full set of that user's permissions. An insecure program can do just as much damage as a trusted one.

***

### 2.4. Bootstrapping Authentication
A critical problem is establishing an identity for the very first process when a new user logs in. Since the user had no processes running previously, this new process cannot inherit a User ID from a parent. The system must have a mechanism to create a new process and assign it the correct identity after the user proves who they are.

***

### 2.5. Authentication Methods

#### 2.5.1. Passwords
*   **Authentication based on "what you know":** A user proves their identity by providing a secret string of characters that only they should know.
*   **Storage mechanism:** To check a password, the system needs a stored reference. Storing a **hash** of the password is much more secure than storing the password in plaintext. If an attacker steals the hash file, they cannot easily reverse the hash to get the password, whereas stealing a plaintext password file grants them immediate access.
*   **Common problems:**
    *   **Brute-force attacks:** Modern computers are so fast that they can rapidly guess millions of short passwords.
    *   **Network sniffing:** If a password is sent over an unencrypted network, an attacker can intercept it.
    *   **The conflict between being unguessable and memorable:** Passwords must be complex enough to prevent guessing but simple enough for a human to remember.
*   **Best practices for password security:** Based on guidance from agencies like NIST, the focus has shifted.
    *   **Current advice:** Passwords should be long, unguessable, never written down, and never shared.
    *   **Deprecated advice:** Requirements for special characters and frequent password changes are no longer seen as effective. This is because users often make predictable changes (e.g., `Password1`, `Password2`) that provide little real security benefit.

***

#### 2.5.2. Challenge/Response Systems
*   **Authentication based on correctly answering a challenge:** The system presents a challenge (a question), and the user must provide the correct response.
*   **Hardware-Based Challenge/Response:** This method is based on **"what you have"** and is much more secure.
    *   The challenge is sent to a physical hardware device that the user possesses, like a smartphone or a smart card (e.g., an ATM card with a chip).
    *   The device performs a secret computation on the challenge (which is different every time) and sends back a unique response.
    *   An example is UCLA's Duo system, where a push notification is sent to a registered phone.
*   **Problems:**
    *   Knowledge-based systems (like "What was your first pet's name?") are very weak, as the answers are often easily discoverable online.
    *   Hardware-based systems fail if you lose the device. If the device is stolen, the thief might be able to impersonate you.

***

#### 2.5.3. Biometric Authentication
*   **Authentication based on "what you are":** This method uses a unique physical characteristic of a person for identification. Examples include fingerprints, facial recognition, retinal patterns, and voiceprints.
*   **Process:**
    1. A hardware sensor (e.g., a fingerprint reader) measures the physical attribute.
    2. The measurement is converted into a binary representation.
    3. The system checks if this representation is a **close match** to a previously stored value for that user. A bit-for-bit comparison is not possible due to natural variations in measurement (e.g., finger placement, lighting, a user having a cold).
*   **Problems:**
    *   Requires special hardware (cameras, scanners).
    *   Physical traits can vary (e.g., a voice can be hoarse).
    *   Highly vulnerable to replay attacks across a network. An attacker who captures the binary representation of a valid fingerprint can simply re-transmit it to authenticate without having the physical finger.
*   **Inaccuracy types:**
    *   **False Positives:** The system incorrectly identifies an unauthorized user as an authorized one. This happens if the matching algorithm is too generous.
    *   **False Negatives:** The system fails to identify an authorized user. This happens if the matching algorithm is too picky.

***

#### 2.5.4. Multi-factor Authentication (MFA)
*   **The use of two or more independent authentication methods:** MFA combines factors from different categories (e.g., "what you know" + "what you have"). A common example is requiring a password and a code sent to your phone.
*   **Compensates for the drawbacks of individual methods:** The goal is for the different factors to cover each other's weaknesses. For an attacker to succeed, they would have to defeat both mechanisms.
*   **The current preferred approach to authentication:** MFA provides significantly stronger security than any single-factor method.

***

## 3. Access Control

### 3.1. The Role of Access Control in Operating Systems
Once a user has been authenticated, the OS must decide what they are allowed to do. **Access control** is the set of mechanisms used to enforce security policies on which processes can access which resources. This check typically happens after a system call is made but before the OS executes the requested operation.

***

### 3.2. Access Control Lists (ACLs)
*   An **Access Control List (ACL)** is a list of permissions attached to an object (like a file).
*   The list specifies which subjects (users or groups) are allowed to perform which actions (e.g., read, write) on that object.
*   When a request is made, the OS checks the object's ACL to see if the subject has the required permission.

#### 3.2.1. Example: The Unix File System
*   The traditional Unix file permission system is a simple, early form of an ACL, developed in the 1970s and still widely used.
*   The "ACL" is stored in the file's **inode** (on-disk descriptor).
*   Due to limited space in early inodes, it's a highly abbreviated system:
    *   **Three subjects:** The file's **Owner**, users in the file's **Group**, and **Other** (everyone else).
    *   **Three modes:** **Read (r)**, **Write (w)**, and **Execute (x)**.
*   This results in the 9 permission bits commonly seen in commands like `ls -l`.

#### 3.2.2. Visual Aid: An ASCII representation of Unix permissions.
```
  --- --- ---
  rwx rwx rwx  (read, write, execute permissions)
   |   |   |
   |   |   +---- Other (all other users)
   |   +-------- Group (users in the file's group)
   +------------ Owner (the file's owner)
```

#### 3.2.3. Pros and Cons of ACLs
*   **Pros:** It is very easy to look at a resource and determine exactly who is allowed to access it. Revoking permission is also straightforward (just remove the entry from the list).
*   **Cons:** It is very difficult to determine all the resources a specific subject can access. To answer "What files can Bob write to?", you would have to scan the ACL of every file on the entire system.

***

### 3.3. Capabilities
*   A **capability** is an unforgeable token that acts like a "ticket" or a key.
*   **Possession of the capability implies permission:** If a process holds a capability for a resource, it is granted the access rights specified by that capability, without needing to check an ACL.
*   **Unforgeability is critical:** Since a capability is just a data structure (a collection of bits), it must be unforgeable. This is typically achieved by having the trusted OS be the sole manager of capabilities. A user process cannot create or modify its own capabilities.
*   **Pros:**
    *   It is very easy to determine what a subject can access by simply listing their held capabilities.
    *   It provides an excellent model for transferring limited privileges (**Principle of Least Privilege**). A parent process can give a child process only a subset of its own capabilities, restricting what the child can do.
*   **Cons:**
    *   It is difficult to determine who can access a specific object, as you'd have to check the capability lists of all subjects.
    *   Revoking a capability is a complex problem.

***

### 3.4. Hybrid OS Use of Access Control
Modern operating systems often use a hybrid approach, combining the strengths of both ACLs and capabilities.

#### 3.4.1. Example: Unix/Linux File Access
1.  When a process calls `open()` on a file, the OS performs an **ACL check** using the file's `rwx` permissions to see if the user is authorized.
2.  If the check succeeds, the OS creates an **in-memory file descriptor** and returns it to the process.
3.  This file descriptor then acts as a **capability**. For all subsequent `read()` or `write()` calls using that descriptor, the OS only needs to check that the descriptor is valid and allows the requested operation (e.g., read-only), without having to re-check the on-disk ACL every time.

***

### 3.5. Enforcing Access in an OS
For any access control model to work, the OS must have a robust way to enforce it.

*   **Inaccessibility by Default:** Protected resources must be made inaccessible to user processes by default. This is enforced using **hardware protection** mechanisms that prevent user-mode code from directly accessing physical resources.
*   **The OS as the Gatekeeper:** Only the OS, running in a privileged kernel mode, can make these resources accessible.
*   **Requesting Access via System Calls:** A process must issue a **system call** to the OS to request access. This transition from user mode to kernel mode gives the OS a chance to mediate the request.
*   **Policy Check:** Upon receiving the system call, the OS consults its access control policy data (e.g., an ACL) to determine if the request should be granted.
*   **Granting Access:** If authorized, access can be granted in two main ways:
    *   **Directly:** The OS maps the resource directly into the process's address space (e.g., a **memory-mapped file**). The process can then access it with normal memory instructions without further OS intervention for each access.
    *   **Indirectly:** The OS returns a handle, such as a file descriptor, which acts as a **capability** for the resource. The process must use this capability in subsequent system calls to perform operations.

***

## 4. Cryptography
Much of computer security is about **keeping secrets**. Cryptography is the primary tool used to achieve this. Its goal is to transform data into a form that is incomprehensible to unauthorized parties, while still allowing authorized users to convert it back to its original, understandable form. This is accomplished by transforming bit patterns in controlled, mathematical ways to gain security advantages.

***

### 4.1. Cryptography Terminology
Cryptography is typically described using a model of sending a message from a **sender (S)** to a **receiver (R)**. The goal is to protect the message from anyone who might intercept it.

*   **Encryption (E):** The process of converting the original message into an unreadable form, making it incomprehensible to anyone except the intended receiver. The algorithm that performs this is often denoted `E()`.
*   **Decryption (D):** The reverse process of converting the unreadable message back into its original, readable form. The algorithm is denoted `D()`.
*   **Cipher:** The specific set of rules or the algorithm used to perform the encryption and decryption transformations.
*   **Cryptosystem:** The complete system that includes the encryption algorithm, the decryption algorithm, and the methods for generating and managing keys.
*   **Plaintext (P):** The original, unencrypted message or data.
*   **Ciphertext (C):** The encrypted, unreadable form of the message.
*   **Key (K):** A secret piece of information (a string of bits) used by the cipher to encrypt and decrypt data.
    *   The security of a modern cryptosystem relies on the secrecy of the key, not the secrecy of the algorithm itself.
    *   The key's primary advantage is that it reduces the problem of keeping a potentially very long message secret to the much simpler problem of keeping a relatively short key secret.
*   The relationship between these terms can be formally expressed as: `C = E(K, P)`.

#### 4.1.1. Visual Aid: Plaintext to Ciphertext Transformation.
```
Plaintext:  "Transfer $100 to my savings account"
     |
     V  E(K, P)
Ciphertext: "Sqzmredq #099 sn lx rzuhmfr zbbntms"
```

***

### 4.2. Symmetric Cryptosystems
*   In a symmetric system, the **same key** is used for both encryption and decryption: `P = D(K, E(K,P))`.
*   **Brute Force Attacks:** This involves trying every possible key until the correct one is found. The security of a symmetric cipher against this attack depends directly on its key length.
    *   A 56-bit DES key (`2^56` possibilities) can be broken in minutes by modern computers.
    *   A 256-bit AES key (`2^256` possibilities) is computationally infeasible to break with current technology.
*   **Popular Ciphers:** **DES** (insecure) and **AES** (current standard).

***

#### 4.2.1. Pros and Cons of Symmetric Cryptosystems
*   **Pros:**
    *   **Speed:** Symmetric algorithms are extremely fast and computationally efficient, making them perfect for encrypting large volumes of data, like streaming video or large file transfers.
    *   **Implicit Authentication:** If two parties share a secret key, a message that decrypts successfully implicitly authenticates the sender. Since only the intended sender knew the key, only they could have created a message that the receiver could decrypt.
*   **Cons:**
    *   **Key Distribution Problem:** This is the most significant weakness. Before two parties can communicate securely, they must find a way to agree on and securely exchange a secret key. This is very difficult to do over an insecure channel like the internet.
    *   **Scalability:** In a system with `N` users who all need to communicate securely in pairs, you need `N * (N-1) / 2` keys. This number grows quadratically and quickly becomes unmanageable.
    *   **Lack of Non-Repudiation:** Because both parties share the exact same secret key, either one of them could have created an encrypted message. This makes it impossible to prove to a third party (like a judge) who originated a message. One party can always claim the other party framed them.

***

### 4.3. Asymmetric (Public Key) Cryptosystems
*   An asymmetric system uses a **pair of keys** for each user: a **public key**, which is shared with everyone, and a **private key**, which is kept secret by the owner.
*   Encryption and decryption use different keys: `P = D(K_private, E(K_public, P))`. What is encrypted with one key can only be decrypted with the other key in the pair.

***

#### 4.3.1. Pros and Cons of Asymmetric Cryptosystems
*   **Pros:**
    *   **Solves Key Distribution:** It elegantly solves the key distribution problem for secret keys. To communicate, you only need to obtain the recipient's public key, which does not need to be kept secret.
    *   **Provides Non-Repudiation:** Because only the owner has the private key, a message encrypted with it serves as an undeniable **digital signature**. The sender cannot later claim they didn't send the message.
    *   **Scalability:** Each user only needs to manage their own single key pair, regardless of how many people they communicate with.
*   **Cons:**
    *   **Speed:** Asymmetric algorithms are extremely slow and computationally expensive (often 100 to 1,000 times slower than symmetric ones). They are impractical for encrypting large amounts of data.
    *   **Public Key Authenticity Problem:** While you don't need to keep public keys secret, you absolutely must ensure they are *authentic*. How do you know the public key for "Amazon.com" really belongs to Amazon and not an attacker impersonating them? This requires a trusted third party, known as a **Certificate Authority (CA)**, to vouch for the identity of public key owners.

***

#### 4.3.2. Popular Asymmetric Ciphers
*   **RSA (Rivest–Shamir–Adleman):**
    *   **Principle:** The most popular and historically significant public key algorithm. Its security is based on the immense computational difficulty of **factoring very large numbers**. It is easy to multiply two large prime numbers to get a product, but extremely hard to take that product and find the original prime factors. This is the "trapdoor" that makes the private key derivable only with secret knowledge (the prime factors).
    *   **Characteristics:** RSA's security is directly tied to its key length. As computers get faster, the required key length increases. For example, a 768-bit key was once secure but is now breakable. The current standard is 2048 bits or higher. This need for very long keys makes RSA operations computationally expensive.
*   **Elliptic Curve Cryptography (ECC):**
    *   **Principle:** A newer alternative to RSA that bases its security on the mathematical properties of elliptic curves and the difficulty of solving the elliptic-curve discrete logarithm problem.
    *   **Characteristics:** ECC's primary advantage is **efficiency**. It can provide the same level of security as RSA but with much smaller key sizes. For example, a 256-bit ECC key is roughly equivalent in strength to a 3072-bit RSA key. This makes it much faster and ideal for systems with limited computational power, such as smartphones and IoT devices. It is now extremely widely used in modern web protocols like TLS.

***

### 4.4. Combined Use of Cryptosystems
In practice, most secure communication sessions (like HTTPS for web browsing) use a combination of both symmetric and asymmetric cryptography to get the best of both worlds.

1.  **Establish Identity and Share a Key:** The session begins by using slow, expensive **asymmetric cryptography**. The client and server use their public/private keys to authenticate each other and securely negotiate a new, temporary key for this session only. This key is called a **session key**.
2.  **Encrypt Bulk Data:** Once the shared session key is established, they switch to fast, efficient **symmetric cryptography** (like AES) using that session key to encrypt all the actual data exchanged during the rest of the session.

***

#### 4.4.1. Example: The Alice and Bob session key exchange model
This simplified example illustrates how the two cryptosystems can be combined to achieve a secure key exchange.
*   **Goal:** Alice wants to send a new, randomly generated symmetric session key (`Ks`) to Bob. The exchange must be **confidential** (only Bob can learn `Ks`) and **authenticated** (Bob must be certain the key came from Alice).
*   **Prerequisites:** Alice and Bob must have authentic copies of each other's public keys.
*   **Alice's Actions (Creating and Sending the Message):**
    1.  **Ensure Confidentiality:** Alice encrypts the session key `Ks` using **Bob's public key**. This creates an inner ciphertext layer. Because it's encrypted with Bob's public key, only the holder of Bob's private key (Bob) can ever decrypt it.
    2.  **Ensure Authenticity (Signing):** Alice takes the result from step 1 (the encrypted session key) and encrypts it *again* using **her own private key**. This creates an outer "digital signature" layer. Since only Alice possesses her private key, this action proves that she is the originator of the message.
    3.  Alice sends the final, double-encrypted message to Bob.
*   **Bob's Actions (Receiving and Verifying the Message):**
    1.  **Verify Authenticity:** Bob receives the message and first uses **Alice's public key** to decrypt the outer layer. If this succeeds, it mathematically proves the message was encrypted with Alice's private key, confirming it came from Alice.
    2.  **Retrieve the Secret:** The result of step 1 is the inner ciphertext (the session key still encrypted with Bob's public key). Bob now uses **his own private key** to decrypt this layer, successfully recovering the original session key, `Ks`.
*   **Outcome:** Alice and Bob now share the secret session key `Ks` and can proceed to communicate using fast symmetric encryption.
*   **Important Caveat:** This specific "encrypt-then-sign" textbook example is conceptually useful but has known security flaws. Modern protocols like TLS use more robust and complex methods to achieve the same goals. This example's purpose is to illustrate the *concept* of combining the two cryptographic types.

***

## 5. Conclusion
*   **Security is an immense and critical problem in modern computing.** Flaws can have severe real-world consequences.
*   **The operating system's security is fundamental.** Because the OS sits at the base of the software stack, its security is critical for protecting everything that runs on top of it.
*   **Authentication answers "who is asking?"** The OS must be able to reliably determine the identity of the process requesting an action.
*   **Access control answers "should this be allowed?"** Based on the authenticated identity, the OS uses mechanisms like ACLs and capabilities to enforce security policies.
*   **Cryptography is an essential tool for implementing security protections.** It provides the fundamental building blocks for confidentiality, authentication, and integrity, both within a single machine and across distributed systems.
