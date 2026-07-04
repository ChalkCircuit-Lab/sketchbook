package uaic.dbxdrgsl.PrefSchedule.dto;

import uaic.dbxdrgsl.PrefSchedule.model.UserRole;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@NoArgsConstructor
@AllArgsConstructor
public class RegisterRequest {
    private String username;
    private String password;
    private String email;
    private String firstName;
    private String lastName;
    private UserRole role;
    private String studentNumber;
    private String group;
    private String department;
    private String specialization;
}
