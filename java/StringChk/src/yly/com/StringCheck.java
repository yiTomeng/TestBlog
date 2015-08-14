package yly.com;

import java.util.Map;

import yly.newcom.StringNew;

public class StringCheck {
    
    public static void main(String[] args) {
        /* test 1
        String i = "world";
        String s = "hello" + i + ",everyone";
        
        StringBuffer sBuf = new StringBuffer();
        sBuf.append("hello").append(i).append(",everyone");
        
        System.out.println(s);
        System.out.println(sBuf);
        */
        
        /* test 2
        String a = "TtblStart-desc";
        if (a.matches("^[a-zA-Z]*-(asc|desc)$")) {
            System.out.println("a");
        }else{
            System.out.println("b");
        }
        */
        
        /* test 3
        Map<String, String> testMap = new HashMap<String, String>();
        testMap.put("1", "a");
        testMap.put("2", "b");
        
        System.out.println("変更前:");
        System.out.println(testMap.get("1"));
        
        StringCheck check = new StringCheck();
        check.testMapFunc(testMap);
        
        System.out.println("変更後:");
        System.out.println(testMap.get("1"));
        */
        
        /* test3 normalizer
        String test = "123１２３abcａｂｃあいうアイウｱｲｳが";
        
        System.out.println("NFC:");
        System.out.println(Normalizer.normalize(test, Normalizer.Form.NFC));
        
        System.out.println("NFKC:");
        System.out.println(Normalizer.normalize(test, Normalizer.Form.NFKC));
        
        System.out.println("NFD:");
        System.out.println(Normalizer.normalize(test, Normalizer.Form.NFD));
        
        System.out.println("NFKD:");
        System.out.println(Normalizer.normalize(test, Normalizer.Form.NFKD));
        */
        
        /* test4 calendar
        Calendar now = Calendar.getInstance();
        now.add(Calendar.DAY_OF_MONTH, +1);
        System.out.println(now.getTime());
        SimpleDateFormat fmt = new SimpleDateFormat("yyyyMMdd");
        System.out.println(fmt.format(now.getTime()));
        */
        
        StringCheck a = new StringCheck();
        StringNew b = new StringNew();
        
        System.out.print("StringCheck:  ");
        System.out.println(a.getClass().getClassLoader());
        
        System.out.print("StringNew:  ");
        System.out.println(b.getClass().getClassLoader());
        
        return;
    }
    
    public void testMapFunc(Map<String, String>map){
        map.replace("1", "ab");
    }
}
