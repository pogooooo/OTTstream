
import { useNavigate } from 'react-router-dom';
import { useSelector, useDispatch } from 'react-redux';
import styled from 'styled-components';
import type {RootState} from '../Store';
import { logout } from '../slices/UserSlice';

const Header = () => {
    const navigate = useNavigate();
    const dispatch = useDispatch();

    // Redux 상태에서 로그인 정보 가져오기
    const isLoggedIn = useSelector((state: RootState) => state.user.isLoggedIn);
    const nickname = useSelector((state: RootState) => state.user.user?.nickname);

    const handleLogout = () => {
        dispatch(logout());
        navigate('/signIn');
    };

    return (
        <HeaderWrapper>
            <Title onClick={() => navigate('/')}>My OTT Stream</Title>
            <Nav>
                {isLoggedIn ? (
                    <>
                        <UserStatus>
                            안녕하세요, **{nickname}**님
                        </UserStatus>
                        <Button onClick={handleLogout}>로그아웃</Button>
                    </>
                ) : (
                    <>
                        <Button onClick={() => navigate('/signIn')}>로그인</Button>
                        <Button onClick={() => navigate('/signUp')}>회원가입</Button>
                    </>
                )}
            </Nav>
        </HeaderWrapper>
    );
};

const HeaderWrapper = styled.header`
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 15px 30px;
    background-color: #202020;
    color: white;
    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.5);
`;

const Title = styled.h1`
    font-size: 24px;
    cursor: pointer;
    margin: 0;
`;

const Nav = styled.nav`
    display: flex;
    gap: 15px;
    align-items: center;
`;

const UserStatus = styled.span`
    font-size: 14px;
    color: #a0a0a0;
`;

const Button = styled.button`
    padding: 8px 15px;
    background-color: #e50914; /* 넷플릭스 스타일 */
    color: white;
    border: none;
    border-radius: 4px;
    cursor: pointer;
    font-weight: bold;
    &:hover {
        background-color: #f40612;
    }
`;

export default Header;
