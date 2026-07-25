# STCP UDP selective NACK recovery v9

This version is based on v7 (connection-ID routing) and adds explicit selective
loss recovery.

- PacketType::Nack carries the first missing sequence in the sequence field.
- An out-of-order receiver sends NACK immediately, then repeats it every four
  later out-of-order frames while the same gap remains.
- The sender retransmits exactly the requested pending frame immediately.
- Timer-based bounded retransmit remains as fallback.
- Events:
  - 314: NACK sent (arg0 = missing sequence)
  - 315: NACK requested a frame no longer pending
  - 316: selective frame retransmitted
