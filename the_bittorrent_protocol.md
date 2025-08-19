## Abstract
This is basically my understanding of the bittorrent protocol as i work on building a client for the protocol.

## The BitTorrent Protocol
The bittorrent protocol is a protocol for distributing files. It advantage over plain HTTP is that when multiple download of a file happen concurrently, the downloaders upload to each other[1].

## Bencoding
Bencoding, to my understanding, is the way the bittorrent protocol maps its data. It represents strings, Integers Lists and dictionaries are so[1]
* **Strings** are size prefixed, with a colon separating the prefix from the string itself
* **Integers** are prefixed by `i` and followed by its value and the postfixed with `e`, bencoded integers have no size limits
* **Lists** are prefixed by `l` followed by they also bencoded elements
* **Dictionaries** are prefixed with `d`, followed by a list of alternating keys and alternating values[1]
```python
4:spam              # a string of size 4 holding the value "spam"
i10e                # an Integer of value 10
l5:felix4:spami20e  # list -> ["felix", "spam", 20]
d3:agei22e          # dict -> {"age" : 22}
```

## What is a Tracker
A tracker is simply an HTTP/HTTPS service that responds to HTTP GET requests.
The request include metrics from the client to help the tracker have an overall statistics about the torrent.The response is a peer list that enables the client participate in the torrent[2]

## Torrent files / Metainfo files (.torrent)
.torrent files are basically bencoded dictionaries with the following keys:
1) `announce`: The URL of the tracker
2) `info`: mapped to a dictionary, with the below keys
	1) `name`: This key maps to a UTF-8 encoded string which is the suggested name to save the file or directory as (optional)
	2) `piece length`: it maps to the number of bytes each piece of the file is split into, except the last piece (which would most likely be truncated). length is usually a power of 2.[1]
	3) `pieces`: maps string whose length is a multiple of 20, it is to be subdivided to strings of length 20, each which is the SHA1 hash of the piece at the corresponding index.[1, 2]
	4) key `length` or a key `files`: both existence are mutually exclusive, but either of them must, always, be present. If `length` is present then the download represents a single file otherwise it represents a directory with  a set of files
	5) `private`: its optional, its an integer field. if the value is 1, private mode, the client MUST publish its presence to the  metainfo-defined trackers to get the trackers peers list. Once its presence is acknowledged it can get a peer list that shows only the peers the defined trackers have access to. If the field is set to `0`, the client may obtain peers from another means. This understanding is somewhat hand-wavy.[2]
3) `announce-list` : (optional) for backwards compatibility[2]
4) `creation date`: (optional) name and version of program used to create the .torrent[2]
5) `encoding`: (optional) string encoding format used to generate the `pieces` part[2]

## Single file cases and Multi file cases
In single file cases; `info` key `length` maps to the length of the file in bytes[1].

In the case of Multi file cases (directories with files or such), the multi file constructs is treated as single file by concatenating the files in the order they appear in the file list[1], The file list is the value `files` maps to, it is a list of dictionaries containing the following keys:
* `length`: length of file in bytes[1, 2]
* `path`: list of UTF-8 encoded strings corresponding to sub-directory names, the last of which is the actual file name (a zero length list is an error case)[1]. An example, the file "dir1/dir2/file.ext", would consist of three string elements bencoded as `l4:dir14:dir2:8:file.exte`[2]

## Notes Regarding the `piece length`
The **piece length** specifies the nominal piece size, and is usually a power of 2. It's value is typically dependent on the total size of data the torrent file is representing. A value too large would lead to inefficacy and one to small would yield large .torrent metadata files[2]. 

Best practice is to keep the piece size 512KB or less, for torrents around 8-10GB[2]. The most common sizes are 256KB, 512KB and 1MB[2].

Every piece is of equal length except the final piece, due to truncation[2], Also each piece has its corresponding SHA1 hash and each of those hashes are concatenated to form the value of the `pieces` key in the `info` dictionary, the `pieces` key is not a list of hashes rather its a long string of concatenated hashes in which each 20th index is the start of a new consecutive hash; string range `[0-20)` of the string is the first hash[2].

## Tracer HTTP/HTTPS Protocol
The tracker is basically a web service which responds to HTTP `GET` commands. It receives parameters from its clients that helps it track the state of the torrent. It response is a peer list that enables the client participate in the torrent[2].

The base URL consists of the `announce URL` as defined in the metainfo file, the parameters added to it via standard CGI methods. All binary data in the URL must be properly escaped, Meaning any bye not in the set[2] :
```python
{[0-9], [a-z], [A-Z], "'", "-", "_", "-"}
```
Must be encoded using the "%nn" format, where nn is the hexadecimal value of the byte[2]. Will get back to this.

## Tracker Request Parameters
A Trackers `GET` request have the following keys:
* `info_hash`:  This is a SHA1 has of the `info` value dictionary in the metainfo file, meaning the hashed dictionary must be equivalent to bdecoding the metainfo file, extracting the info dictionary and bencoding its back *if and only if* the decode validates the metainfo file as a valid .torrent file (follows key ordering, absence of leading zeros and such). Meaning clients must only either reject invalid metainfo files or extract the `info` dictionary substring. A client must not perform a decode - encode roundtrip on invalid data.[1]
* `peer_id`: A string of length 20 which this downloader uses as its id. Each downloader generates its own id at random at the start of a new download. This value must also be escaped. It serves as a unique id for the client and may even be binary data, There is not guideline for generating a `peer_id`, though one would not be wrong to assume that your clients id must at leas be unique for your local machine, This should probably incoperate things like process ID and perhaps timestamp recorded at startup[1,2].
* `ip`: This is an optional parameter giving the Ip (or dns name) which this peer (you the client) is at. Generally used for the origin if its on the same machine as the tracker.[1] Its is usually represents in dotted quad format or rfc3513 defined hexed IPV6 address.[1, 2]
* `port`: This is the port this peer is listening on. Common behavior is for the downloader to try to listen on port 6881 and if that port is taken try 6882, then 6883 and give up after 6889.[1]
* `uploaded`: The total amount uploaded so far, encoded in base ten ascii[1]
* `downloaded`: This is the total amount downloaded so far, in base ten ascii[1]
* `left`: This  is the number of bytes this peer still has to downloaded encoded in base ten ascii. This can't be computed from the difference of the `downloaded` to the file  `length` since it might be a resume and some of the downloaded data failed the integrity check and and had to be re-downloaded.
* `event`: This is an optional key which maps to`started`, ``completed`` or `stopped`, (or `empty`, which is the same as not being present). if not present, this is one of the announcements done at regular intervals. An `announcement` using started is sent  when the file first begins, and one using `completed` is sent when the download is complete. No `complete` is sent if the file was complete when started. Downloaders send an announcement using `stopeed` wen they cease downloading.[1]
* `compact`: setting this to 1 indicates that the clients accepts a compact response: when this is toggled, the peer list is replaced by a peers string with 6  bytes per peer, First four bytes are the host (in network byte order), the last two bytes are the port (also in network byte order). Some trackers only support compact responses (saves bandwidth), either they refuse requests without "compact=1" or simply send a compact response unless the request unless the request contains "compact=0", here if "compact=0" was explicitly stated the tracker will refuse the request[2].
* `numwant`: This key is optional, It is the number of peers that the client would like to receive from the tracker. This value is permitted to be zero. If omitted, typically defaults to 50 peers.[2]
* `key`: This is optional. It is an additional identification that is not share with other peers. It is intended to allow a client prove their identity should their IP address change[2].
* `trackerid`: Also optional. If a previous announce contained a tracker id. I should be set here

## Trackers Response
Trackers responses are bencoded dictionaries, if a trackers response has a key `failure reason`, That maps to a human readable string that explains why the request failed, and no other keys are required. If that is not the case, then it must have two keys; `interval` and `peers`, The key`interval` maps to the number of seconds the downloader should wait between regular requests. and `peers` maps to a list of dictionaries corresponding to peers, Each of which contains the keys:
* `peer_id`: The peers self selected id
* `ip`: IP address or DNS name as a string
* `port`, the port numbers to listen for incoming data.
Other keys in the responses:
* `min interval`: Optional, This is the minimum announce interval, if present clients mist not re-announce more frequently than this[2]
* `complete`: Number of peers with the entire flle, i.e seeders (integer)
* `incomplete`: number of non-seeders, aka "leechers" (integer)
* `peers` (Binary model, available in compact mode, i'm guessing) This key values to a string consisting of multiples of 6 bytes. First 4 bytes are the IP address and last 2 Bytes are the port number. All in network (big endian) notation

"Announcements" are common over a UDP tracker protocol

List of peers is 50 by default, could be smaller if there are fewer peers on the torrent the list will be smaller, Otherwise, the tracker randomly selects peers to include in the response. Client can request from trackers more than the specifed interval, if an even occures (completed, stopped or such), or if the client wants to learn more about its peers, But it is considered bad practice to "hammer" on the tracker by requesting at intervals more frequent than what the trackers demands, just to know about more peers, If a client wants more peers, the satisfactory number of peers should have been declared in the `numwant` parameter while sending a request.[2]

Even 30 peers is plenty, The value of the number of peers is crucial to performance, when a new piece is downloaded the client (downloader) would have to send `HAVE` messages to the most active peers, and as the number of peers grows, the overhead of having to keep communication to multiple peers and,possibly, at the same time continue downloaded remaning pieces can have an effect on performance, According to wiki theory, peers above 25 are unlikely to even increase download speed.[2]

The cost of broadcast traffic grows in direct proportion to the number of peers[2].
## Trackers "Scrape" Convention
By convention trackers offer another form of request, a scrape request, It returns a zipped compressed file or "text/plain" holding a bencoded dictionary. The content of this dictionary is the overall statistics of all torrent files the tracker manages. We can determine if a tracker supports scrape requests if after analyzing the `announce` URL in the meta-info file, the last `/` in the URL is followed by, exactly, the string "announce" and only that, If that is the case we can make a scrape request by replacing the "announce" at the end of the URL with "scrape" and then sending a GET request to the newly modified url.

We can even get more specific on the exact torrent we seek statistics for (instead of the whole catalog of torrents the tracker manages) by sending the request with `info_hash` parameters appended to the scrape url, The `info_hash`  parameter must hold the value of the `SHA1` hash of the `info` dictionary of the required torrent. we can also scrape multiple numbers of `info_hash` specified torrents:
```HTTP
GET http://example.com/scrape.php?info_hash=aaaa..aa&info_hash=bbb..bb
```
where `aaaa...aa` (such similar to it) is a 20 byte info hashed string of the required torrent.

scrape requests helps reduce the load and bandwidth of a tracker when all we want is scrap statistics.

Here is the content of the returned bencoded dictionary:
* **files**: a dictionary containing one key/value pair for each torrent for which there are stats, if the `info_hash` supplied was valid, the dictionary will only contain key/value pair entries for the specifically requested torrents. Each key consists of a 20-byte binary *info_hash* its value would contain the following:
	* **complete**: number of peers with the entire file (seeders)
	* **downloaded**: total number of times the tracker has registered a completion ("event=complete") 
	* **incomplete**: number of non-seeder peers (leechers)
	* **name**: optional, the torrents internal name, as specified by the "name" file in the info section of the .torrent file.

## Unofficial extensions to scrape
These are response keys being unofficially used, since they are unofficial, they are all optional. 
* **failure reason**: Human readable error, message regarding whey the request failed
* **flags**: A dictionary containing miscellaneous flags. The value of the flags key is another nested dictionary, possibly containing the following:
	* **min_request_interval**: minimum time in seconds the client should wait before scraping the tracker again. 

## Peer protocol
BitTorrent peer protocol operates over TCP or uTP. The connection between peers are symmetrical, they are identical in representation and data can flow in either direction.[1]

The peer protocol refers to pieces of the files by index described by the metainfo file, the index starts at zero, Once a client or peer or downloader, finishes downloading a piece, it checks for hash matching and once successful, announces to other peers participating in the torrent that it has that piece of the file.[1]

Connections contain two bits of state on either end:
* Choked or not Chocked
* Interested or not Interested.[1]
Choking is a notification that no data will be sent until unchoking happens, the reason for chokes will be explained later . 

## When Does Transfer occur?
Data transfer takes place whenever one side is interested and the other side is not choking. interests state must be kept up to date at all times. When a downloader doesn't have something they would had immediately asked a peer for in unchoked, they must express their lack of interest, despite being choked. This can be tricky in implementation, but it males it possible fro downloaders to know which peers will start downloading immediately if unchoked.

Peer to peer connections start our chocked and not interested. When data is being transferred, downloaders must should keep several piece requests on queue to ensure good TCP performance, this is known as pipelining[1].

The peer wire protocol consists of a handshake followed by a never ending stream of length-prefixed messages.  A handshake starts with the character 19 (decimal), followed by the string "BitTorrent protocol"[1], the leading character is the length prefix. All later integers sent in the protocol are encoded as 4 bytes big endian.

After the fixed headers, are 8 reserved bytes which are all valued at 0 in all current implementation. Next comes the 20 byte `sha1` hashed of the bencoded form of the info value in the metainfo file ( same value which is announced as `info_hash` to the tracker, only in this case its raw instead of quoted), If both sides don't send the same value, they sever the connection. There is one exception, which is in the case that a downloader wants to perform numerous file downloads on a single port, They might wait for incoming connections to give a download hash, first then check their list and respond with a match[1]

After the download hash comes the peer id (the same one in the tracker requests and contained in peer list of the trackers response). If the receiving sides peer id doesn't match the one the initiating side expects, it severs the connection.

## Peer messages
All non-keep alive messaged start with a single byte which gives their type, possible values:
* 0 - choke
* 1 - unchoke
* 2 - interested
* 3 - unintrested
* 4 - have
* 5 - bitfield
* 6 - request
* 7 - piece
* 8 - cancel

the first four values have no payload. 'bitfield' is only ever sent as the first message. Its payload is a bitfield with each index of the file the downloader has sent set to one and the rest set to zero. If a downloader has nothing yet  may skip the bitfield message. The first byte of of the bitfield corresponds to index 0-7, from high bit to low bit, The next byte index 8-15, etc, Spare bits at the end are set to zero.

The "have" messages payload is a single number of the most recent index the downloader just downloaded and have successfully checked the hash of.

"request" messages contain an index, begin, and length. The last two are byte offsets. Length is generally a power of 2 unless it's the end of the file where its length might be truncated. All current implementation use `2^14`(16kb), and close connections which requests amount greater than that.

"cancel" messages have the same payload as request messages. They are usually sent at the end  of the download, which is known as "endgame mode". In most scenarios during the ending stage of a download where a downloader only requires few pieces, The tendency of the downloader to haul on a single hosed modem is quite likely, so a way to combat this is that, once the downloader has a few pieces left to download with its requests still pending from its requested source, it sent a request for all the pieces to all peers its downloading from, And to prevent this tactic from becoming horribly inefficient, for each piece it successfully obtains it sends a "cancel" request of that piece to every peer.[1]

"piece" messages contain an index, begin, and piece, It should be noted that they are correlated with request messages implicitly. it is possible for an unexpected piece to be sent if a choke and  unchoke messages are sent in rapid succession and/or if transfer is going very slowly.

Downloaders generally download pieces in random order, which does a reasonably good job of keeping them from having a strict subset or superset of the pieces of any of their peers.

## Why do we choke?
Choking is done for many reasons, one of them is that TCP congestion operates very badly with numerous connections happening simultaneously. Also choking enables peers using an "acceptable" algorithm to maintain a consistent download rate.[1]

## What should a good Choking algorithm be?
It should cap the numbers of simultaneous uploads for good TCP performance. It should prevent rapid choking and unchoking, also known as fibrillation, It should reciprocate to peers that let it download, and it should once in while check unused peer connections for they could be better than the currently connected ones, this approach is known as optimistic unchoking.

Bram Cohen's, which i suspect has a team that develops the official/standard implementation of the BitTorrent protocol, says that the currently deployed choking algorithm, avoid fibrillation, by only changing who's choked once every ten seconds. It perfroms reciprocation and number of upload capping  by unchocking the four peers that it notices it's best downloads rates from and are currently interested. Peers with better upload rates but aren't interested get unchoked and if they do become interested, the worst uploader gets choked. If a downloader has a complete file, it uses its upload rate rather than download to rate to decide who to unchoke.

In optimistic unchoking, at any one time there is a single peer which is unchoked regardless of it's upload rate (if its interested, it counts as one of the 4 allowed downloaders). Which peer is optimistically unchoked rotates every 30 seconds. To give them a them a decent chance of getting a complete piece to upload.

## Peer wire protocol (TCP)
The peer protocol facilitates transfers of blocks between peers as described in the metainfo file. A client must maintain state information **FOR EACH CONNECTION** that it has with a remote. A client, i believe, must actively be aware of the current state of a connection it has with a remote peer, This would be a continuous action. 

The "state" that must be maintained, relative to the client, embodies this two data:
* **choked**: Whether or not the remote peer has choked this client. When a remote peer chokes a client it is signal that the remote peer would not be answering any request the client makes to the peer. When this occurs, the client should refrain from sending any request for blocks to the peer until unchoked, It should also consider all pending request to the peer at that time as disregarded by the remote peer.
* **interested**: This signals whether or not the remote peer is interested in what the client has to offer. This is a notification that the remote peer will begin requesting blocks when the client unchokes them.

Note this also implies that the client would also have to keep track whether it itself is **interested**  or not in whatever the remote peer has to offer and also if the the client has that peer **chocked** or not, so the complete schema of the state the client would have to keep track of for each connection to a peer, would looks something similar to this: 
* **am_choking**: this client is choking the peer.
* **am_interested**: This client is interested in what the peer has to offer
* **peer_choking**: This client is being choked by the remote peer
* **peer_interested**: What this client has to offer is of interest to the remote peer

Client connections start out as choked and no interested, in other words, would look something similar to this in c++:
```c++
struct Connection_state {
	// default values represent client ot peer initial connection state
	uint8_t am_intrested = 1; 
	uint8_t am_intrested = 0;
	uint8_t peer_choking = 1;
	uint8_t peer interested = 0; 
};
// begin of connection
Connection_state me_and_peer1 {}; 
```
A block is downloaded by the client when the client is interested in a peer, and that peer is not choking the client. A c
## References
1.  The Official bittorrent specification for developers
2. [Wiki-theory](https://wiki.theory.org/Bithttps://www.bittorrent.org/bittorrentecon.pdfTorrentShttps://wiki.theory.org/Bithttps://www.bittorrent.org/bittorrentecon.pdfTorrentSpecificationpecification