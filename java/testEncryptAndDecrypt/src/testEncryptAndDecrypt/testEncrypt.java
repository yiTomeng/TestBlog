package testEncryptAndDecrypt;

import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESKeySpec;

public class testEncrypt {

    public static void main(String[] args) {
        // TODO 自動生成されたメソッド・スタブ
        testEncrypt testObject = new testEncrypt();
        
        String key = "12345678";
        String beforeEncryptStr = "abcdefgh";
        byte[] afterEncryptStr = testObject.testEncryptFunc(beforeEncryptStr, key);
        byte[] afterDecryptStr = testObject.testDecryptFunc(afterEncryptStr, key);
        /*
        try {
            //System.out.println(new String(afterEncryptStr, "UTF-8"));
            for (byte a : afterEncryptStr) {
                System.out.println(a);
            }
            System.out.println(new String(afterDecryptStr, "UTF-8"));
            
        } catch (UnsupportedEncodingException e) {
            // TODO 自動生成された catch ブロック
            e.printStackTrace();
        }
        */
        //System.out.println(Base64.encodeBase64String(afterEncryptStr);
        System.out.println(afterDecryptStr);
    }

    public byte[] testEncryptFunc(String data, String key) {
        // DES　random数字を作成  
        SecureRandom sr = new SecureRandom();  
        //キーパラメータからDESKeySpecのオブジェクトを作る  
        DESKeySpec dks;
        try {
            dks = new DESKeySpec(key.getBytes());
            // FactoryからDESKeySpecをSecretKeyに変換
            SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("DES");  
            SecretKey securekey = keyFactory.generateSecret(dks);  
            // Cipher　インスタンス取得
            Cipher cipher = Cipher.getInstance("DES");  
            // Cipherのオブジェクト初期化  
            cipher.init(Cipher.ENCRYPT_MODE, securekey, sr);  
              
            // 暗号化する  
            return cipher.doFinal(data.getBytes());
        } catch (InvalidKeyException | NoSuchAlgorithmException | InvalidKeySpecException | NoSuchPaddingException | IllegalBlockSizeException | BadPaddingException e) {
            // TODO 自動生成された catch ブロック
            e.printStackTrace();
        }
        return null;  
          
    }
    
    public byte[] testDecryptFunc(byte[] data, String key) {
        // DES　random数字を作成    
        SecureRandom sr = new SecureRandom();  
        // キーパラメータからDESKeySpecのオブジェクトを作る 
        DESKeySpec dks;
        try {
            dks = new DESKeySpec(key.getBytes());
            // FactoryからDESKeySpecをSecretKeyに変換
            SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("DES");  
            SecretKey securekey = keyFactory.generateSecret(dks);  
            // Cipher　インスタンス取得
            Cipher cipher = Cipher.getInstance("DES");  
            // Cipher　インスタンス取得
            cipher.init(Cipher.DECRYPT_MODE, securekey, sr);  
            // 暗号化する   
            return cipher.doFinal(data); 
        } catch (InvalidKeyException | NoSuchAlgorithmException | InvalidKeySpecException | NoSuchPaddingException | IllegalBlockSizeException | BadPaddingException e) {
            // TODO 自動生成された catch ブロック
            e.printStackTrace();
        }  
         
        return null;
    }
}
