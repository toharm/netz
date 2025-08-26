# NeTz

NeTz comes from Hawk (נץ) in Hebrew.

Across social media, several bot accounts are created promoting donations to obscure cryptocurrencies. 
While these accounts promise that the money will reach suffering people, I was suspicious that this money is actually just being 
funneled directly to terror organizations and not to people in need.

[Ethplorer](https://ethplorer.io/) has several such addresses that silently collect funds for terrorism under the guise of humanitarian aid.

Note that, while this is the original use case, NeTz can be used to track any address on supported blockchains deemed suspicious based on sanctions.

## Use

### API Keys
[TronGrid](https://www.trongrid.io/) - Tron transaction information

[Shyft](https://shyft.to/get-api-key) - Solana transaction information

[Etherscan](https://etherscan.io/) - Ethereum transaction information

[Chainalysis](https://www.chainalysis.com/) - Sanctioned address querying

### Running the Application
```bash
g++ -std=c++17 -DCPPHTTPLIB_OPENSSL_SUPPORT -I/opt/homebrew/opt/openssl/include -L/opt/homebrew/opt/openssl/lib src/*.cpp -lssl -lcrypto -o netz
```
Example command:
```bash
./netz --threads 4 --network ethereum --target 0x123abc...
```
