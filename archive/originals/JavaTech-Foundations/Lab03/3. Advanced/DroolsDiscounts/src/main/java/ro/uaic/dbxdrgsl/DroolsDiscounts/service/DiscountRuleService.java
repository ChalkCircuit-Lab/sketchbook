package ro.uaic.dbxdrgsl.DroolsDiscounts.service;

import org.kie.api.runtime.KieContainer;
import org.kie.api.runtime.KieSession;
import org.springframework.stereotype.Service;
import ro.uaic.dbxdrgsl.DroolsDiscounts.model.Order;

@Service
public class DiscountRuleService {
  private final KieContainer kieContainer;
  public DiscountRuleService(KieContainer kieContainer){ this.kieContainer = kieContainer; }

  public Order applyDiscount(Order order) {
    KieSession ks = kieContainer.newKieSession("discountSession");
    try {
      ks.insert(order);
      ks.fireAllRules();
      return order;
    } finally {
      ks.dispose();
    }
  }
}
